#include "detour.h"
#include "server/server.h"
#include "offsets.h"

constexpr auto CMD_ENGAGEDUEL_F_OFFSET = 0x0012F834;
constexpr auto G_ENTITIES_OFFSET = 0x006CE620;
constexpr auto G_GAMETYPE_OFFSET = 0x00685A80;
constexpr auto TRAP_SENDSERVERCOMMAND_OFFSET = 0x0015E454;
constexpr auto GET_STRING_ED_OFFSET = 0x0008B184;
constexpr auto PATCH = "\x33\xC0\xC3"; // xor eax, eax, ret
constexpr auto G_OTHERPLAYERSDUELING_OFFSET = 0x0012F304;
constexpr auto CLIENT_THINK_REAL_OFFSET = 0x00011C4C4;
constexpr auto CLIENT_THINK_REAL_START_LOCATION = 0x0011CE05;
constexpr auto CLIENT_THINK_REAL_RETURN_LOCATION = 0x11D0B5;

static void *g_entities = nullptr;
static vmCvar_t *g_gametype = nullptr;
static void (*trap_SendServerCommand)(int, const char *) = nullptr;
static const char *(*G_GetStringEdString)(char *refSection, char *refName) = nullptr;
static uintptr_t g_base = 0;
static qboolean (*OnSameTeam)(void *a, void *b) = nullptr;
static void (*G_AddEvent)(void *ent, int event, int eventParm) = nullptr;
static void (*G_Sound)(void *ent, int channel, int soundIndex) = nullptr;
static cvar_t *g_duelHealth = nullptr;
static cvar_t *g_duelArmor = nullptr;

static void holster_saber(void *ent) {
	if (!client_ps(ent_client(ent))->saberHolstered) {
		if (client_saber(ent_client(ent))[0].soundOff) {
			G_Sound(ent, CHAN_AUTO, client_saber(ent_client(ent))[0].soundOff);
		}
		if (client_saber(ent_client(ent))[1].soundOff
			&& client_saber(ent_client(ent))[1].model[0]) {
			G_Sound(ent, CHAN_AUTO, client_saber(ent_client(ent))[1].soundOff);
		}
		client_ps(ent_client(ent))->weaponTime = 400;
		client_ps(ent_client(ent))->saberHolstered = 2;
	}

}

static void DuelActive(void *ent) {
	Com_Printf("DuelActive\n");
}

static void EngageDuel(void *ent) {
	if (g_gametype->integer == GT_DUEL || g_gametype->integer == GT_POWERDUEL) { //rather pointless in this mode..
		trap_SendServerCommand(client_num(g_base, ent), va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "NODUEL_GAMETYPE")));
		return;
	}

	if (g_gametype->integer >= GT_TEAM) { //no private dueling in team modes
		trap_SendServerCommand(client_num(g_base, ent), va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "NODUEL_GAMETYPE")));
		return;
	}

	if (client_ps(ent_client(ent))->duelTime >= sv.time
	    || client_ps(ent_client(ent))->weapon != WP_SABER
	    || client_ps(ent_client(ent))->saberInFlight
	    || client_ps(ent_client(ent))->duelInProgress) {
		return;
	}

	if (client_ps(ent_client(ent))->fd.privateDuelTime > sv.time) {
		trap_SendServerCommand(client_num(g_base, ent), va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "CANTDUEL_JUSTDID")) );
		return;
	}

	vec3_t fwd, fwdOrg;
	AngleVectors(client_ps(ent_client(ent))->viewangles, fwd, NULL, NULL);
	fwdOrg[0] = client_ps(ent_client(ent))->origin[0] + fwd[0] * 256;
	fwdOrg[1] = client_ps(ent_client(ent))->origin[1] + fwd[1] * 256;
	fwdOrg[2] = (client_ps(ent_client(ent))->origin[2] + client_ps(ent_client(ent))->viewheight) + fwd[2] * 256;

	trace_t tr;
	SV_Trace(&tr, client_ps(ent_client(ent))->origin, NULL, NULL, fwdOrg, ent_s(ent)->number, MASK_PLAYERSOLID, 0, 0, 10);

	if (tr.fraction != 1
	    && tr.entityNum < MAX_CLIENTS
	    && tr.entityNum >= 0) {
		void *challenged = gentity_for_num(g_base, tr.entityNum);
		if (!ent_client(challenged)
		    || !ent_inuse(challenged)
		    || ent_health(challenged) < 1
			|| client_ps(ent_client(challenged))->stats[STAT_HEALTH] < 1
		    || client_ps(ent_client(challenged))->weapon != WP_SABER
		    || client_ps(ent_client(challenged))->duelInProgress
		    || client_ps(ent_client(challenged))->saberInFlight) {
			return;
		}
		if (g_gametype->integer >= GT_TEAM
		    && OnSameTeam(ent, challenged)) {
			return;
		}
		if (client_ps(ent_client(challenged))->duelIndex == ent_s(ent)->number
		    && client_ps(ent_client(challenged))->duelTime >= sv.time) {
			trap_SendServerCommand(-1, va("print \"%s^7 %s %s^7!\n\"", client_pers(ent_client(challenged))->netname, G_GetStringEdString("MP_SVGAME", "PLDUELACCEPT"), client_pers(ent_client(ent))->netname));

			client_ps(ent_client(ent))->duelInProgress = qtrue;
			client_ps(ent_client(challenged))->duelInProgress = qtrue;

			client_ps(ent_client(ent))->duelTime = sv.time + 2000;
			client_ps(ent_client(challenged))->duelTime = sv.time + 2000;

			G_AddEvent(ent, EV_PRIVATE_DUEL, 1);
			G_AddEvent(challenged, EV_PRIVATE_DUEL, 1);

			//Holster their sabers now, until the duel starts (then they'll get auto-turned on to look cool)
			holster_saber(ent);
			holster_saber(challenged);

			ent_set_health(ent, g_duelHealth->integer);
			ent_set_health(challenged, g_duelHealth->integer);

			client_ps(ent_client(ent))->stats[STAT_HEALTH] = g_duelHealth->integer;
			client_ps(ent_client(challenged))->stats[STAT_HEALTH] = g_duelHealth->integer;

			client_ps(ent_client(ent))->stats[STAT_ARMOR] = g_duelArmor->integer;
			client_ps(ent_client(challenged))->stats[STAT_ARMOR] = g_duelArmor->integer;
		} else {
			// Print the message that a player has been challenged in private, only announce the actual duel initiation in private
			trap_SendServerCommand(client_num(g_base, challenged), va("cp \"%s^7 %s\n\"", client_pers(ent_client(ent))->netname, G_GetStringEdString("MP_SVGAME", "PLDUELCHALLENGE")));
			trap_SendServerCommand(client_num(g_base, ent), va("cp \"%s %s^7\n\"", G_GetStringEdString("MP_SVGAME", "PLDUELCHALLENGED"), client_pers(ent_client(challenged))->netname));
		}

		client_ps(ent_client(challenged))->fd.privateDuelTime = 0; //reset the timer in case this player just got out of a duel. He should still be able to accept the challenge.

		client_ps(ent_client(ent))->forceHandExtend = HANDEXTEND_DUELCHALLENGE;
		client_ps(ent_client(ent))->forceHandExtendTime = sv.time + 1000;

		client_ps(ent_client(ent))->duelIndex = ent_s(challenged)->number;
		client_ps(ent_client(ent))->duelTime = sv.time + 5000;
	}
}


namespace jampog {
	static void patch_client_think(uintptr_t base) {
		uintptr_t CUR = base + CLIENT_THINK_REAL_START_LOCATION;
		// pushad (backup registers incase they are clobbered)
		patch_byte((unsigned char*)CUR, 0x60);
		CUR += 1;
		// push esi (gentity_t *ent)
		patch_byte((unsigned char*)CUR, 0x56);
		CUR += 1;
		// call DuelActive
		patch_byte((unsigned char*)CUR, 0xE8);
		patch_word((unsigned int*)(CUR+1), (unsigned int)((uintptr_t)DuelActive - (CUR + 5)));
		CUR += 5;
		// pop esi
		patch_byte((unsigned char*)CUR, 0x5E);
		CUR += 1;
		// popad (restore registers)
		patch_byte((unsigned char*)CUR, 0x61);
		CUR += 1;
		// jmp RET_LOC
		uintptr_t RET_LOC = (base + CLIENT_THINK_REAL_RETURN_LOCATION) - (CUR + 5);
		patch_byte((unsigned char*)CUR, 0xE9);
		patch_word((unsigned int*)(CUR+1), RET_LOC);
	}
	void patch_engage_duel(uintptr_t base) {
		g_base = base;
		detour((void*)(base + CMD_ENGAGEDUEL_F_OFFSET), (void*)EngageDuel);
		//patch_str((void*)(base + G_OTHERPLAYERSDUELING_OFFSET), PATCH);
		g_entities = (void*)(base + G_ENTITIES_OFFSET);
		trap_SendServerCommand = (decltype(trap_SendServerCommand))(base + TRAP_SENDSERVERCOMMAND_OFFSET);
		g_gametype = (vmCvar_t*)(base + G_GAMETYPE_OFFSET);
		G_GetStringEdString = (decltype(G_GetStringEdString))(base + GET_STRING_ED_OFFSET);
		OnSameTeam = (decltype(OnSameTeam))(base + 0x001638E4);
		G_AddEvent = (decltype(G_AddEvent))(base + 0x0016E564);
		G_Sound = (decltype(G_Sound))(base + 0x0016E824);
		g_duelHealth = Cvar_Get("g_duelHealth", "100", CVAR_ARCHIVE);
		g_duelArmor = Cvar_Get("g_duelArmor", "100", CVAR_ARCHIVE);

		//patch_client_think(base);
	}
}
