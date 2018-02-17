#include "detour.h"
#include "server/server.h"

constexpr auto CMD_ENGAGEDUEL_F_OFFSET = 0x0012F834;
constexpr auto G_ENTITIES_OFFSET = 0x006CE620;
constexpr auto G_GAMETYPE_OFFSET = 0x00685A80;
constexpr auto TRAP_SENDSERVERCOMMAND_OFFSET = 0x0015E454;
constexpr auto GET_STRING_ED_OFFSET = 0x0008B184;
constexpr auto PATCH = "\x33\xC0\xC3"; // xor eax, eax, ret
constexpr auto G_OTHERPLAYERSDUELING_OFFSET = 0x0012F304;

static void *g_entities = nullptr;
static vmCvar_t *g_gametype = nullptr;
static void (*trap_SendServerCommand)(int, const char *) = nullptr;
static const char *(*G_GetStringEdString)(char *refSection, char *refName) = nullptr;

static void EngageDuel(void *ent) {
	//trace_t tr;
	//vec3_t forward, fwdOrg;

	const int client_num = (uintptr_t)ent - (uintptr_t)g_entities;

	if (g_gametype->integer == GT_DUEL || g_gametype->integer == GT_POWERDUEL) { //rather pointless in this mode..
		trap_SendServerCommand(client_num, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "NODUEL_GAMETYPE")));
		return;
	}

	if (g_gametype->integer >= GT_TEAM) { //no private dueling in team modes
		trap_SendServerCommand(client_num, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "NODUEL_GAMETYPE")));
		return;
	}
#if 0
	if (ent->client->ps.duelTime >= level.time) {
		return;
	}

	if (ent->client->ps.weapon != WP_SABER) {
		return;
	}

	if (ent->client->ps.saberInFlight) {
		return;
	}

	if (ent->client->ps.duelInProgress) {
		return;
	}

	if (ent->client->ps.fd.privateDuelTime > level.time) {
		SV_SendServerCommand(client_num, va("print \"%s\n\"", SV_GetStringEdString("MP_SVGAME", "CANTDUEL_JUSTDID")) );
		return;
	}

	AngleVectors( ent->client->ps.viewangles, forward, NULL, NULL );

	fwdOrg[0] = ent->client->ps.origin[0] + forward[0]*256;
	fwdOrg[1] = ent->client->ps.origin[1] + forward[1]*256;
	fwdOrg[2] = (ent->client->ps.origin[2]+ent->client->ps.viewheight) + forward[2]*256;

	SV_Trace(&tr, ent->client->ps.origin, NULL, NULL, fwdOrg, ent->s.number, MASK_PLAYERSOLID);

	if (tr.fraction != 1 && tr.entityNum < MAX_CLIENTS) {
		gentity_t *challenged = g_entities[tr.entityNum];
		if (!challenged || !challenged->client || !challenged->inuse ||
			challenged->health < 1 || challenged->client->ps.stats[STAT_HEALTH] < 1 ||
			challenged->client->ps.weapon != WP_SABER || challenged->client->ps.duelInProgress ||
			challenged->client->ps.saberInFlight) {
			return;
		}
		if (g_gametype->integer >= GT_TEAM && OnSameTeam(ent, challenged)) {
			return;
		}
		if (challenged->client->ps.duelIndex == ent->s.number && challenged->client->ps.duelTime >= level.time) {
			SV_SendServerCommand(-1, va("print \"%s %s %s!\n\"", challenged->client->pers.netname, SV_GetStringEdString("MP_SVGAME", "PLDUELACCEPT"), ent->client->pers.netname));

			ent->client->ps.duelInProgress = qtrue;
			challenged->client->ps.duelInProgress = qtrue;

			ent->client->ps.duelTime = level.time + 2000;
			challenged->client->ps.duelTime = level.time + 2000;

			G_AddEvent(ent, EV_PRIVATE_DUEL, 1);
			G_AddEvent(challenged, EV_PRIVATE_DUEL, 1);

			//Holster their sabers now, until the duel starts (then they'll get auto-turned on to look cool)

			if (!ent->client->ps.saberHolstered)
			{
				if (ent->client->saber[0].soundOff)
				{
					G_Sound(ent, CHAN_AUTO, ent->client->saber[0].soundOff);
				}
				if (ent->client->saber[1].soundOff &&
					ent->client->saber[1].model[0])
				{
					G_Sound(ent, CHAN_AUTO, ent->client->saber[1].soundOff);
				}
				ent->client->ps.weaponTime = 400;
				ent->client->ps.saberHolstered = 2;
			}
			if (!challenged->client->ps.saberHolstered)
			{
				if (challenged->client->saber[0].soundOff)
				{
					G_Sound(challenged, CHAN_AUTO, challenged->client->saber[0].soundOff);
				}
				if (challenged->client->saber[1].soundOff &&
					challenged->client->saber[1].model[0])
				{
					G_Sound(challenged, CHAN_AUTO, challenged->client->saber[1].soundOff);
				}
				challenged->client->ps.weaponTime = 400;
				challenged->client->ps.saberHolstered = 2;
			}
		}
		else
		{
			//Print the message that a player has been challenged in private, only announce the actual duel initiation in private
			SV_SendServerCommand(challenged-g_entities, va("cp \"%s %s\n\"", ent->client->pers.netname, SV_GetStringEdString("MP_SVGAME", "PLDUELCHALLENGE")) );
			SV_SendServerCommand(client_num, va("cp \"%s %s\n\"", SV_GetStringEdString("MP_SVGAME", "PLDUELCHALLENGED"), challenged->client->pers.netname) );
		}

		challenged->client->ps.fd.privateDuelTime = 0; //reset the timer in case this player just got out of a duel. He should still be able to accept the challenge.

		ent->client->ps.forceHandExtend = HANDEXTEND_DUELCHALLENGE;
		ent->client->ps.forceHandExtendTime = level.time + 1000;

		ent->client->ps.duelIndex = challenged->s.number;
		ent->client->ps.duelTime = level.time + 5000;
	}
	#endif
}

namespace jampog {
	void patch_engage_duel(uintptr_t base) {
		// detour((void*)(base + CMD_ENGAGEDUEL_F_OFFSET), (void*)EngageDuel);
		patch_str((void*)(base + G_OTHERPLAYERSDUELING_OFFSET), PATCH);
		g_entities = (void*)(base + G_ENTITIES_OFFSET);
		trap_SendServerCommand = (decltype(trap_SendServerCommand))(base + TRAP_SENDSERVERCOMMAND_OFFSET);
		g_gametype = (vmCvar_t*)(base + G_GAMETYPE_OFFSET);
		G_GetStringEdString = (decltype(G_GetStringEdString))(base + GET_STRING_ED_OFFSET);
	}
}
