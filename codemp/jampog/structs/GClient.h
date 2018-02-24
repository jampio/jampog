#pragma once

namespace jampog {
	typedef enum {
		TEAM_FREE,
		TEAM_RED,
		TEAM_BLUE,
		TEAM_SPECTATOR,

		TEAM_NUM_TEAMS
	} Team;
	typedef enum {
		SPECTATOR_NOT,
		SPECTATOR_FREE,
		SPECTATOR_FOLLOW,
		SPECTATOR_SCOREBOARD
	} SpectatorState;
	struct ClientSession {
		Team		sessionTeam;
		int			spectatorTime;		// for determining next-in-line to play
		SpectatorState	spectatorState;
		int			spectatorClient;	// for chasecam and follow mode
		int			wins, losses;		// tournament stats
		int			selectedFP;			// check against this, if doesn't match value in playerstate then update userinfo
		int			saberLevel;			// similar to above method, but for current saber attack level
		qboolean	setForce;			// set to true once player is given the chance to set force powers
		int			updateUITime;		// only update userinfo for FP/SL if < level.time
		qboolean	teamLeader;			// true when this client is a team leader
		char		siegeClass[64];
		char		saberType[64];
		char		saber2Type[64];
		int			duelTeam;
		int			siegeDesiredTeam;
		int			killCount;
		int			TKCount;
		char		IPstring[32];		// yeah, I know, could be 16, but, just in case...
	};
	enum {
		CON_DISCONNECTED,
		CON_CONNECTING,
		CON_CONNECTED
	};
	typedef int ClientConnected;
	typedef enum {
		TEAM_BEGIN,		// Beginning a team game, spawn at base
		TEAM_ACTIVE		// Now actively playing
	} PlayerTeamStateState;
	struct PlayerTeamState {
		PlayerTeamStateState	state;
		int			location;
		int			captures;
		int			basedefense;
		int			carrierdefense;
		int			flagrecovery;
		int			fragcarrier;
		int			assists;
		float		lasthurtcarrier;
		float		lastreturnedflag;
		float		flagsince;
		float		lastfraggedcarrier;
	};
	struct ClientPersistant {
		ClientConnected	connected;
		usercmd_t	cmd;				// we would lose angles if not persistant
		qboolean	localClient;		// true if "ip" info key is "localhost"
		qboolean	initialSpawn;		// the first spawn should be at a cool location
		qboolean	predictItemPickup;	// based on cg_predictItems userinfo
		qboolean	pmoveFixed;			//
		char		netname[MAX_NETNAME];
		int			netnameTime;				// Last time the name was changed
		int			maxHealth;			// for handicapping
		int			enterTime;			// level.time the client entered the game
		PlayerTeamState teamState;	// status in teamplay games
		int			voteCount;			// to prevent people from constantly calling votes
		int			teamVoteCount;		// to prevent people from constantly calling votes
		qboolean	teamInfo;			// send team overlay updates?
	};
	#pragma pack(push, 1)
	struct GClient {
		playerState_t	ps;
		ClientPersistant	pers;
		ClientSession		sess;
		saberInfo_t	saber[MAX_SABERS];
		void		*weaponGhoul2[MAX_SABERS];
		int			tossableItemDebounce;
		int			bodyGrabTime;
		int			bodyGrabIndex;
		int			pushEffectTime;
		int			invulnerableTimer;
		int			saberCycleQueue;
		int			legsAnimExecute;
		int			torsoAnimExecute;
		qboolean	legsLastFlip;
		qboolean	torsoLastFlip;
		qboolean	readyToExit;
		qboolean	noclip;
	};
	#pragma pack (pop)
}
#if 0

	int			lastCmdTime;		// level.time of last usercmd_t, for EF_CONNECTION
									// we can't just use pers.lastCommand.time, because
									// of the g_sycronousclients case
	int			buttons;
	int			oldbuttons;
	int			latched_buttons;

	vec3_t		oldOrigin;

	// sum up damage over an entire frame, so
	// shotgun blasts give a single big kick
	int			damage_armor;		// damage absorbed by armor
	int			damage_blood;		// damage taken out of health
	int			damage_knockback;	// impact damage
	vec3_t		damage_from;		// origin for vector calculation
	qboolean	damage_fromWorld;	// if true, don't use the damage_from vector

	int			damageBoxHandle_Head; //entity number of head damage box
	int			damageBoxHandle_RLeg; //entity number of right leg damage box
	int			damageBoxHandle_LLeg; //entity number of left leg damage box

	int			accurateCount;		// for "impressive" reward sound

	int			accuracy_shots;		// total number of shots
	int			accuracy_hits;		// total number of hits

	//
	int			lastkilled_client;	// last client that this client killed
	int			lasthurt_client;	// last client that damaged this client
	int			lasthurt_mod;		// type of damage the client did

	// timers
	int			respawnTime;		// can respawn when time > this, force after g_forcerespwan
	int			inactivityTime;		// kick players when time > this
	qboolean	inactivityWarning;	// qtrue if the five seoond warning has been given
	int			rewardTime;			// clear the EF_AWARD_IMPRESSIVE, etc when time > this

	int			airOutTime;

	int			lastKillTime;		// for multiple kill rewards

	qboolean	fireHeld;			// used for hook
	gentity_t	*hook;				// grapple hook if out

	int			switchTeamTime;		// time the player switched teams

	int			switchDuelTeamTime;		// time the player switched duel teams

	int			switchClassTime;	// class changed debounce timer

	// timeResidual is used to handle events that happen every second
	// like health / armor countdowns and regeneration
	int			timeResidual;

	char		*areabits;

	int			g2LastSurfaceHit; //index of surface hit during the most recent ghoul2 collision performed on this client.
	int			g2LastSurfaceTime; //time when the surface index was set (to make sure it's up to date)

	int			corrTime;

	vec3_t		lastHeadAngles;
	int			lookTime;

	int			brokenLimbs;

	qboolean	noCorpse; //don't leave a corpse on respawn this time.

	int			jetPackTime;

	qboolean	jetPackOn;
	int			jetPackToggleTime;
	int			jetPackDebRecharge;
	int			jetPackDebReduce;

	int			cloakToggleTime;
	int			cloakDebRecharge;
	int			cloakDebReduce;

	int			saberStoredIndex; //stores saberEntityNum from playerstate for when it's set to 0 (indicating saber was knocked out of the air)

	int			saberKnockedTime; //if saber gets knocked away, can't pull it back until this value is < level.time

	vec3_t		olderSaberBase; //Set before lastSaberBase_Always, to whatever lastSaberBase_Always was previously
	qboolean	olderIsValid;	//is it valid?

	vec3_t		lastSaberDir_Always; //every getboltmatrix, set to saber dir
	vec3_t		lastSaberBase_Always; //every getboltmatrix, set to saber base
	int			lastSaberStorageTime; //server time that the above two values were updated (for making sure they aren't out of date)

	qboolean	hasCurrentPosition;	//are lastSaberTip and lastSaberBase valid?

	int			dangerTime;		// level.time when last attack occured

	int			idleTime;		//keep track of when to play an idle anim on the client.

	int			idleHealth;		//stop idling if health decreases
	vec3_t		idleViewAngles;	//stop idling if viewangles change

	int			forcePowerSoundDebounce; //if > level.time, don't do certain sound events again (drain sound, absorb sound, etc)

	char		modelname[MAX_QPATH];

	qboolean	fjDidJump;

	qboolean	ikStatus;

	int			throwingIndex;
	int			beingThrown;
	int			doingThrow;

	float		hiddenDist;//How close ents have to be to pick you up as an enemy
	vec3_t		hiddenDir;//Normalized direction in which NPCs can't see you (you are hidden)

	renderInfo_t	renderInfo;

	//mostly NPC stuff:
	npcteam_t	playerTeam;
	npcteam_t	enemyTeam;
	char		*squadname;
	gentity_t	*team_leader;
	gentity_t	*leader;
	gentity_t	*follower;
	int			numFollowers;
	gentity_t	*formationGoal;
	int			nextFormGoal;
	class_t		NPC_class;

	vec3_t		pushVec;
	int			pushVecTime;

	int			siegeClass;
	int			holdingObjectiveItem;

	//time values for when being healed/supplied by supplier class
	int			isMedHealed;
	int			isMedSupplied;

	//seperate debounce time for refilling someone's ammo as a supplier
	int			medSupplyDebounce;

	//used in conjunction with ps.hackingTime
	int			isHacking;
	vec3_t		hackingAngles;

	//debounce time for sending extended siege data to certain classes
	int			siegeEDataSend;

	int			ewebIndex; //index of e-web gun if spawned
	int			ewebTime; //e-web use debounce
	int			ewebHealth; //health of e-web (to keep track between deployments)

	int			inSpaceIndex; //ent index of space trigger if inside one
	int			inSpaceSuffocation; //suffocation timer

	int			tempSpectate; //time to force spectator mode

	//keep track of last person kicked and the time so we don't hit multiple times per kick
	int			jediKickIndex;
	int			jediKickTime;

	//special moves (designed for kyle boss npc, but useable by players in mp)
	int			grappleIndex;
	int			grappleState;

	int			solidHack;

	int			noLightningTime;

	unsigned	mGameFlags;

	//fallen duelist
	qboolean	iAmALoser;

	int			lastGenCmd;
	int			lastGenCmdTime;

	//can't put these in playerstate, crashes game (need to change exe?)
	int			otherKillerMOD;
	int			otherKillerVehWeapon;
	int			otherKillerWeaponType;
};
#endif