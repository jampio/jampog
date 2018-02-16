#include "qcommon/qcommon.h"
#include "detour.h"

constexpr auto NUM_FORCE_MASTERY_LEVELS = 8;
constexpr auto DEFAULT_FORCEPOWERS = "7-1-032330000000001333";
constexpr auto DEFAULT_FORCEPOWERS_LEN = 22;
constexpr auto GT_TEAM = 6;
constexpr auto BG_LegalizedForcePowers = 0x000EA224;

static const int forceMasteryPoints[NUM_FORCE_MASTERY_LEVELS] = {
	0,		// FORCE_MASTERY_UNINITIATED,
	5,		// FORCE_MASTERY_INITIATE,
	10,		// FORCE_MASTERY_PADAWAN,
	20,		// FORCE_MASTERY_JEDI,
	30,		// FORCE_MASTERY_JEDI_GUARDIAN,
	50,		// FORCE_MASTERY_JEDI_ADEPT,
	75,		// FORCE_MASTERY_JEDI_KNIGHT,
	100		// FORCE_MASTERY_JEDI_MASTER,
};

static int forcePowerDarkLight[NUM_FORCE_POWERS] = { //nothing should be usable at rank 0..
	FORCE_LIGHTSIDE,//FP_HEAL,//instant
	0,//FP_LEVITATION,//hold/duration
	0,//FP_SPEED,//duration
	0,//FP_PUSH,//hold/duration
	0,//FP_PULL,//hold/duration
	FORCE_LIGHTSIDE,//FP_TELEPATHY,//instant
	FORCE_DARKSIDE,//FP_GRIP,//hold/duration
	FORCE_DARKSIDE,//FP_LIGHTNING,//hold/duration
	0,//FP_SABERATTACK,
	0,//FP_SABERDEFEND,
	0,//FP_SABERTHROW,
	//new Jedi Academy powers
	FORCE_DARKSIDE,//FP_RAGE,//duration
	FORCE_LIGHTSIDE,//FP_PROTECT,//duration
	FORCE_LIGHTSIDE,//FP_ABSORB,//duration
	FORCE_DARKSIDE,//FP_DRAIN,//hold/duration
	0,//FP_SEE,//duration
	//NUM_FORCE_POWERS
};

static const int bgForcePowerCost[NUM_FORCE_POWERS][NUM_FORCE_POWER_LEVELS] = {
	{	0,	2,	4,	6	},	// Heal			// FP_HEAL
	{	0,	0,	2,	6	},	// Jump			//FP_LEVITATION,//hold/duration
	{	0,	2,	4,	6	},	// Speed		//FP_SPEED,//duration
	{	0,	1,	3,	6	},	// Push			//FP_PUSH,//hold/duration
	{	0,	1,	3,	6	},	// Pull			//FP_PULL,//hold/duration
	{	0,	4,	6,	8	},	// Mind Trick	//FP_TELEPATHY,//instant
	{	0,	1,	3,	6	},	// Grip			//FP_GRIP,//hold/duration
	{	0,	2,	5,	8	},	// Lightning	//FP_LIGHTNING,//hold/duration
	{	0,	4,	6,	8	},	// Dark Rage	//FP_RAGE,//duration
	{	0,	2,	5,	8	},	// Protection	//FP_PROTECT,//duration
	{	0,	1,	3,	6	},	// Absorb		//FP_ABSORB,//duration
	{	0,	1,	3,	6	},	// Team Heal	//FP_TEAM_HEAL,//instant
	{	0,	1,	3,	6	},	// Team Force	//FP_TEAM_FORCE,//instant
	{	0,	2,	4,	6	},	// Drain		//FP_DRAIN,//hold/duration
	{	0,	2,	5,	8	},	// Sight		//FP_SEE,//duration
	{	0,	1,	5,	8	},	// Saber Attack	//FP_SABER_OFFENSE,
	{	0,	1,	5,	8	},	// Saber Defend	//FP_SABER_DEFENSE,
	{	0,	4,	6,	8	}	// Saber Throw	//FP_SABERTHROW,
	//NUM_FORCE_POWERS
};

static
__attribute__ ((__cdecl__))
qboolean legalized_forcepowers(char *powerOut,
                                      int maxRank, qboolean freeSaber,
                                      int teamForce, int gametype,
                                      int fpDisabled) {
	size_t powerOutSize = DEFAULT_FORCEPOWERS_LEN+1;
	char powerBuf[128];
	char readBuf[128];
	qboolean maintainsValidity = qtrue;
	int powerLen = strlen(powerOut);
	size_t i = 0;
	int c = 0;
	int allowedPoints = 0;
	int usedPoints = 0;
	int countDown = 0;
	int final_Side;
	int final_Powers[NUM_FORCE_POWERS] = {0};

	if ( powerLen >= 128 )
	{ //This should not happen. If it does, this is obviously a bogus string.
		//They can have this string. Because I said so.
		Q_strncpyz( powerBuf, DEFAULT_FORCEPOWERS, sizeof( powerBuf ) );
		maintainsValidity = qfalse;
	}
	else
		Q_strncpyz( powerBuf, powerOut, sizeof( powerBuf ) ); //copy it as the original

	//first of all, print the max rank into the string as the rank
	Q_strncpyz( powerOut, va( "%i-", maxRank ), powerOutSize );

	while (i < sizeof( powerBuf ) && powerBuf[i] && powerBuf[i] != '-') {
		i++;
	}

	i++;

	while (i < sizeof( powerBuf ) && powerBuf[i] && powerBuf[i] != '-') {
		readBuf[c] = powerBuf[i];
		c++;
		i++;
	}

	readBuf[c] = 0;
	i++;
	//at this point, readBuf contains the intended side
	final_Side = atoi(readBuf);

	if (final_Side != FORCE_LIGHTSIDE &&
	    final_Side != FORCE_DARKSIDE)
	{ //Not a valid side. You will be dark. Because I said so. (this is something that should never actually happen unless you purposely feed in an invalid config)
		final_Side = FORCE_DARKSIDE;
		maintainsValidity = qfalse;
	}

    if (teamForce)
    { //If we are under force-aligned teams, make sure we're on the right side.
        if (final_Side != teamForce)
        {
            final_Side = teamForce;
            //maintainsValidity = qfalse;
            //Not doing this, for now. Let them join the team with their filtered powers.
        }
    }

    //Now we have established a valid rank, and a valid side.
    //Read the force powers in, and cut them down based on the various rules supplied.
    c = 0;
    while (i < sizeof( powerBuf ) && powerBuf[i] && powerBuf[i] != '\n' && powerBuf[i] != '\r'
           && powerBuf[i] >= '0' && powerBuf[i] <= '3' && c < NUM_FORCE_POWERS)
    {
        readBuf[0] = powerBuf[i];
        readBuf[1] = 0;
        final_Powers[c] = atoi(readBuf);
        c++;
        i++;
    }

    //final_Powers now contains all the stuff from the string
    //Set the maximum allowed points used based on the max rank level, and count the points actually used.
    allowedPoints = forceMasteryPoints[maxRank];

    i = 0;
    while (i < NUM_FORCE_POWERS)
    { //if this power doesn't match the side we're on, then 0 it now.
        if (final_Powers[i] &&
            forcePowerDarkLight[i] &&
            forcePowerDarkLight[i] != final_Side)
        {
            final_Powers[i] = 0;
            //This is only likely to happen with g_forceBasedTeams. Let it slide.
        }

        if ( final_Powers[i] &&
             (fpDisabled & (1 << i)) )
        { //if this power is disabled on the server via said server option, then we don't get it.
            final_Powers[i] = 0;
        }

        i++;
    }

    if (gametype < GT_TEAM)
    { //don't bother with team powers then
        final_Powers[FP_TEAM_HEAL] = 0;
        final_Powers[FP_TEAM_FORCE] = 0;
    }

    usedPoints = 0;
    i = 0;
    while (i < NUM_FORCE_POWERS)
    {
        countDown = Com_Clampi( 0, NUM_FORCE_POWER_LEVELS, final_Powers[i] );

        while (countDown > 0)
        {
            usedPoints += bgForcePowerCost[i][countDown]; //[fp index][fp level]
            //if this is jump, or we have a free saber and it's offense or defense, take the level back down on level 1
            if ( countDown == 1 &&
                 ((i == FP_LEVITATION) ||
                  (i == FP_SABER_OFFENSE && freeSaber) ||
                  (i == FP_SABER_DEFENSE && freeSaber)) )
            {
                usedPoints -= bgForcePowerCost[i][countDown];
            }
            countDown--;
        }

        i++;
    }

    if (usedPoints > allowedPoints)
    { //Time to do the fancy stuff. (meaning, slowly cut parts off while taking a guess at what is most or least important in the config)
        int attemptedCycles = 0;
        int powerCycle = 2;
        int minPow = 0;

        if (freeSaber)
        {
            minPow = 1;
        }

        maintainsValidity = qfalse;

        while (usedPoints > allowedPoints)
        {
            c = 0;

            while (c < NUM_FORCE_POWERS && usedPoints > allowedPoints)
            {
                if (final_Powers[c] && final_Powers[c] < powerCycle)
                { //kill in order of lowest powers, because the higher powers are probably more important
                    if (c == FP_SABER_OFFENSE &&
                        (final_Powers[FP_SABER_DEFENSE] > minPow || final_Powers[FP_SABERTHROW] > 0))
                    { //if we're on saber attack, only suck it down if we have no def or throw either
                        int whichOne = FP_SABERTHROW; //first try throw

                        if (!final_Powers[whichOne])
                        {
                            whichOne = FP_SABER_DEFENSE; //if no throw, drain defense
                        }

                        while (final_Powers[whichOne] > 0 && usedPoints > allowedPoints)
                        {
                            if ( final_Powers[whichOne] > 1 ||
                                 ( (whichOne != FP_SABER_OFFENSE || !freeSaber) &&
                                   (whichOne != FP_SABER_DEFENSE || !freeSaber) ) )
                            { //don't take attack or defend down on level 1 still, if it's free
                                usedPoints -= bgForcePowerCost[whichOne][final_Powers[whichOne]];
                                final_Powers[whichOne]--;
                            }
                            else
                            {
                                break;
                            }
                        }
                    }
                    else
                    {
                        while (final_Powers[c] > 0 && usedPoints > allowedPoints)
                        {
                            if ( final_Powers[c] > 1 ||
                                 ((c != FP_LEVITATION) &&
                                  (c != FP_SABER_OFFENSE || !freeSaber) &&
                                  (c != FP_SABER_DEFENSE || !freeSaber)) )
                            {
                                usedPoints -= bgForcePowerCost[c][final_Powers[c]];
                                final_Powers[c]--;
                            }
                            else
                            {
                                break;
                            }
                        }
                    }
                }

                c++;
            }

            powerCycle++;
            attemptedCycles++;

            if (attemptedCycles > NUM_FORCE_POWERS)
            { //I think this should be impossible. But just in case.
                break;
            }
        }

        if (usedPoints > allowedPoints)
        { //Still? Fine then.. we will kill all of your powers, except the freebies.
            i = 0;

            while (i < NUM_FORCE_POWERS)
            {
                final_Powers[i] = 0;
                if (i == FP_LEVITATION ||
                    (i == FP_SABER_OFFENSE && freeSaber) ||
                    (i == FP_SABER_DEFENSE && freeSaber))
                {
                    final_Powers[i] = 1;
                }
                i++;
            }
            usedPoints = 0;
        }
    }

    if (freeSaber)
    {
        if (final_Powers[FP_SABER_OFFENSE] < 1)
            final_Powers[FP_SABER_OFFENSE] = 1;
        if (final_Powers[FP_SABER_DEFENSE] < 1)
            final_Powers[FP_SABER_DEFENSE] = 1;
    }
    if (final_Powers[FP_LEVITATION] < 1)
        final_Powers[FP_LEVITATION] = 1;

    i = 0;
    while (i < NUM_FORCE_POWERS)
    {
        if (final_Powers[i] > FORCE_LEVEL_3)
            final_Powers[i] = FORCE_LEVEL_3;
        i++;
    }

    if (fpDisabled)
    { //If we specifically have attack or def disabled, force them up to level 3. It's the way
        //things work for the case of all powers disabled.
        //If jump is disabled, down-cap it to level 1. Otherwise don't do a thing.
        if (fpDisabled & (1 << FP_LEVITATION))
            final_Powers[FP_LEVITATION] = 1;
        if (fpDisabled & (1 << FP_SABER_OFFENSE))
            final_Powers[FP_SABER_OFFENSE] = 3;
        if (fpDisabled & (1 << FP_SABER_DEFENSE))
            final_Powers[FP_SABER_DEFENSE] = 3;
    }

    if (final_Powers[FP_SABER_OFFENSE] < 1)
    {
        final_Powers[FP_SABER_DEFENSE] = 0;
        final_Powers[FP_SABERTHROW] = 0;
    }

    //We finally have all the force powers legalized and stored locally.
    //Put them all into the string and return the result. We already have
    //the rank there, so print the side and the powers now.
    Q_strcat(powerOut, powerOutSize, va("%i-", final_Side));

    i = strlen(powerOut);
    c = 0;
    while (c < NUM_FORCE_POWERS)
    {
        Q_strncpyz(readBuf, va( "%i", final_Powers[c] ), sizeof( readBuf ) );
        powerOut[i] = readBuf[0];
        c++;
        i++;
    }
    powerOut[i] = 0;

    return maintainsValidity;
}

namespace jampog {
	void patch_forcepowers(uintptr_t base) {
		detour((void*)(base + BG_LegalizedForcePowers),
		       (void*)legalized_forcepowers);
	}
}
