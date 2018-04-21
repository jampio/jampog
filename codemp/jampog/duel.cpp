#include "detour.h"
#include "server/server.h"
#include "offsets.h"
#include "Entity.h"

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
constexpr auto G_SETTAUNTANIM_OFS = 0x0011BD84;
constexpr auto BG_ANIMLENGTH_OFS = 0x000EEC64;

static void *g_entities = nullptr;
static vmCvar_t *g_gametype = nullptr;
static vmCvar_t *g_spawnInvulnerability = nullptr;
static void (*trap_SendServerCommand)(int, const char *) = nullptr;
static const char *(*G_GetStringEdString)(char *refSection, char *refName) = nullptr;
static uintptr_t g_base = 0;
static qboolean (*OnSameTeam)(sharedEntity_t *a, sharedEntity_t *b) = nullptr;
static void (*G_AddEvent)(void *ent, int event, int eventParm) = nullptr;
static void (*G_Sound)(void *ent, int channel, int soundIndex) = nullptr;
static int (*G_SoundIndex)(const char *s) = nullptr;
static void (*G_SoundOnEnt)(void *ent, int channel, const char *soundPath) = nullptr;
static cvar_t *g_duelHealth = nullptr;
static cvar_t *g_duelArmor = nullptr;
static void (*G_SetTauntAnim)(sharedEntity_t *ent, int) = nullptr;
static int (*BG_Anim_Length)(int, animNumber_t) = nullptr;

#define SEND_DUEL_EVENTS 1

static void holster_saber(sharedEntity_t *ent) {
	if (!ent->playerState->saberHolstered) {
		/* TODO: Causes a crash
		if (client_saber(ent_client(ent))[0].soundOff) {
			G_Sound(ent, CHAN_AUTO, client_saber(ent_client(ent))[0].soundOff);
		}
		if (client_saber(ent_client(ent))[1].soundOff
			&& client_saber(ent_client(ent))[1].model[0]) {
			G_Sound(ent, CHAN_AUTO, client_saber(ent_client(ent))[1].soundOff);
		}
		*/
		ent->playerState->weaponTime = 400;
		ent->playerState->saberHolstered = 2;
	}
}

static void turn_on_saber(sharedEntity_t *ent) {
	ent->playerState->saberHolstered = 0;
	/* TODO: causes a crash
	if (client_saber(ent_client(ent))[0].soundOn) {
		G_Sound(ent, CHAN_AUTO, client_saber(ent_client(ent))[0].soundOn);
	}
	if (client_saber(ent_client(ent))[1].soundOn) {
		G_Sound(ent, CHAN_AUTO, client_saber(ent_client(ent))[1].soundOn);
	}
	*/
}

static void check_begin_duel(sharedEntity_t *ent) {
	if (!(ent->playerState
	     && ent->r.linked
	     && ent->playerState->weapon == WP_SABER
	     && ent->playerState->saberHolstered
	     && ent->playerState->duelTime)) {
		return;
	}
	turn_on_saber(ent);
#if SEND_DUEL_EVENTS
	G_AddEvent(ent, EV_PRIVATE_DUEL, 2);
#endif
	ent->playerState->duelTime = 0;
	svs.clients[SV_NumForGentity(ent)].stats.start_hits();
	// TODO: add begin sound
	// trap_SendServerCommand(client_num(g_base, ent), va("cp \"%s\n\"", G_GetStringEdString("MP_SVGAME", "BEGIN_DUEL")));
	// G_Sound(ent, CHAN_ANNOUNCER, G_SoundIndex("sound/chars/protocol/misc/40MOM038"));
}

#if 0
static void zero_movement(sharedEntity_t *ent) {
	ent->playerState->speed = 0;
	ent->playerState->basespeed = 0;
	auto ps = jampog::Entity(ent).client().persistant();
	ps->cmd.forwardmove = 0;
	ps->cmd.rightmove = 0;
	ps->cmd.upmove = 0;
}
#endif

static void stop_duel_event(sharedEntity_t *ent) {
	ent->playerState->duelInProgress = qfalse;
#if SEND_DUEL_EVENTS
	G_AddEvent(ent, EV_PRIVATE_DUEL, 0);
#endif
}

static void do_bow(sharedEntity_t *ent) {
	holster_saber(ent);
	ent->playerState->forceHandExtend = HANDEXTEND_TAUNT;
	ent->playerState->forceDodgeAnim = BOTH_BOW;
	ent->playerState->forceHandExtendTime = sv.time + BG_Anim_Length(ent->localAnimIndex, BOTH_BOW);
}

static void begin_duel_event(sharedEntity_t *ent) {
	ent->playerState->duelInProgress = qtrue;
	ent->playerState->duelTime = sv.time + 2000;
#if SEND_DUEL_EVENTS
	G_AddEvent(ent, EV_PRIVATE_DUEL, 1);
#endif
	jampog::Entity(ent).set_health(g_duelHealth->integer);
	ent->playerState->stats[STAT_HEALTH] = g_duelHealth->integer;
	ent->playerState->stats[STAT_ARMOR] = g_duelArmor->integer;
	do_bow(ent);
}

static void print_winner_msg(sharedEntity_t *ent, sharedEntity_t *duelAgainst) {
	trap_SendServerCommand(-1, va("print \"%s^7 %s %s^7! ==> ^5HP:^7 (^1%d^7/^2%d^7), ^5hits: ^3%d^7\n\"",
		jampog::Entity(ent).client().name(),
		G_GetStringEdString("MP_SVGAME", "PLDUELWINNER"),
		jampog::Entity(duelAgainst).client().name(),
		ent->playerState->stats[STAT_HEALTH],
		ent->playerState->stats[STAT_ARMOR],
		svs.clients[SV_NumForGentity(ent)].stats.hits()
	));
}

static void print_tie_msg(sharedEntity_t *ent, sharedEntity_t *duelAgainst) {
	trap_SendServerCommand(-1, va("print \"%s^7 %s %s^7\n\"",
		jampog::Entity(ent).client().name(),
		G_GetStringEdString("MP_SVGAME", "PLDUELTIE"),
		jampog::Entity(duelAgainst).client().name()
	));
}

static void reset_health(sharedEntity_t *ent) {
	ent->playerState->stats[STAT_HEALTH] = ent->playerState->stats[STAT_MAX_HEALTH];
	jampog::Entity(ent).set_health(ent->playerState->stats[STAT_MAX_HEALTH]);
	ent->playerState->stats[STAT_ARMOR] = 25;
}

static void DuelActive(sharedEntity_t *ent) {
	if (!ent->playerState->duelInProgress) return;

	sharedEntity_t *duelAgainst = SV_GentityNum(ent->playerState->duelIndex);

	if (ent->playerState->duelTime < sv.time) {
		check_begin_duel(ent);
		check_begin_duel(duelAgainst);
	} else {
		//zero_movement(ent);
	}

	if (auto e = jampog::Entity(duelAgainst);
	    !e.is_player() || !e.inuse() || e.ps().duelIndex != SV_NumForGentity(ent)) {
		stop_duel_event(ent);
	} else if (auto e = jampog::Entity(duelAgainst);
	           e.health() < 1 || e.ps().stats[STAT_HEALTH] < 1) {
		stop_duel_event(ent);
		stop_duel_event(duelAgainst);

		if (jampog::Entity(ent).health() > 0 && ent->playerState->stats[STAT_HEALTH] > 0) {
			print_winner_msg(ent, duelAgainst);
			reset_health(ent);
			if (g_spawnInvulnerability->integer) {
				ent->playerState->eFlags |= EF_INVULNERABLE;
				jampog::Entity(ent).client().set_invulnerable_timer(sv.time + g_spawnInvulnerability->integer);
			}
			// TODO: add some victory sound
			// Q_irand( EV_VICTORY1, EV_VICTORY3 );
			// va("*victory%d.wav", Q_irand(1,3));
		} else {
			print_tie_msg(ent, duelAgainst);
		}
	}
}

static void EngageDuel(sharedEntity_t *ent) {
	if (g_gametype->integer == GT_DUEL || g_gametype->integer == GT_POWERDUEL) { //rather pointless in this mode..
		trap_SendServerCommand(SV_NumForGentity(ent), va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "NODUEL_GAMETYPE")));
		return;
	}

	if (g_gametype->integer >= GT_TEAM) { //no private dueling in team modes
		trap_SendServerCommand(SV_NumForGentity(ent), va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "NODUEL_GAMETYPE")));
		return;
	}

	if (ent->playerState->duelTime >= sv.time
	    || ent->playerState->weapon != WP_SABER
	    || ent->playerState->saberInFlight
	    || ent->playerState->duelInProgress
	    || ent->playerState->weaponTime >= 1) {
		return;
	}

	vec3_t fwd, fwdOrg;
	AngleVectors(ent->playerState->viewangles, fwd, NULL, NULL);
	fwdOrg[0] = ent->playerState->origin[0] + fwd[0] * 256;
	fwdOrg[1] = ent->playerState->origin[1] + fwd[1] * 256;
	fwdOrg[2] = (ent->playerState->origin[2] + ent->playerState->viewheight) + fwd[2] * 256;

	trace_t tr;
	SV_Trace(&tr, ent->playerState->origin, NULL, NULL, fwdOrg, SV_NumForGentity(ent), MASK_PLAYERSOLID, 0, 0, 10);
	if (tr.fraction == 1 || tr.entityNum > MAX_CLIENTS || tr.entityNum < 0) {
		return;
	}

	sharedEntity_t *challenged = SV_GentityNum(tr.entityNum);
	if (!jampog::Entity(challenged).inuse()
		|| !jampog::Entity(challenged).is_player()
		|| jampog::Entity(challenged).health() < 1
		|| ent->playerState->stats[STAT_HEALTH] < 1
		|| ent->playerState->weapon != WP_SABER
		|| ent->playerState->duelInProgress
		|| ent->playerState->saberInFlight) {
		return;
	}

	if (g_gametype->integer >= GT_TEAM
		&& OnSameTeam(ent, challenged)) {
		return;
	}

	ent->playerState->forceHandExtend = HANDEXTEND_DUELCHALLENGE;
	ent->playerState->forceHandExtendTime = sv.time + 1000;
	ent->playerState->duelIndex = SV_NumForGentity(challenged);
	ent->playerState->duelTime = sv.time + 5000;

	if (challenged->playerState->duelIndex == SV_NumForGentity(ent)
	    && challenged->playerState->duelTime >= sv.time) {
		trap_SendServerCommand(-1, va("print \"%s^7 %s %s^7!\n\"", jampog::Entity(challenged).client().name(), G_GetStringEdString("MP_SVGAME", "PLDUELACCEPT"), jampog::Entity(ent).client().name()));
		begin_duel_event(ent);
		begin_duel_event(challenged);
	} else {
		// Print the message that a player has been challenged in private, only announce the actual duel initiation in private
		trap_SendServerCommand(SV_NumForGentity(challenged), va("cp \"%s^7 %s\n\"", jampog::Entity(ent).client().name(), G_GetStringEdString("MP_SVGAME", "PLDUELCHALLENGE")));
		trap_SendServerCommand(SV_NumForGentity(ent), va("cp \"%s %s^7\n\"", G_GetStringEdString("MP_SVGAME", "PLDUELCHALLENGED"), jampog::Entity(challenged).client().name()));
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
		g_spawnInvulnerability = (vmCvar_t*)(base + 0x0084C200);
		G_SoundIndex = (decltype(G_SoundIndex))(base + 0x00170664);
		G_SoundOnEnt = (decltype(G_SoundOnEnt))(base + 0x0016E974);
		G_SetTauntAnim = (decltype(G_SetTauntAnim))(base + G_SETTAUNTANIM_OFS);
		BG_Anim_Length = (decltype(BG_Anim_Length))(base + BG_ANIMLENGTH_OFS);
		Com_Printf("patching ClientThink_real\n");
		patch_client_think(base);
	}
}
