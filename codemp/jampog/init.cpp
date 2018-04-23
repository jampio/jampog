#include "base.h"
#include "forcepowers.h"
#include "followcycle.h"
#include "clean_name.h"
#include "duel.h"
#include "detour.h"
#include "Entity.h"
#include "offsets.h"
#include "icarus/GameInterface.h"
#include "duel_cull.h"
#include "util.h"
#include "damage.h"

namespace console = jampog::console;
using jampog::color_diff;

qboolean cheats_okay(void *ptr);

constexpr auto G_SOUNDTEMPENTITY_OFS = 0x0016E224;
constexpr auto G_MUTESOUND_OFS = 0x0016E7A4;
constexpr auto G_FREEENTITY_OFS = 0x0016DE44;
constexpr auto PMOVE_OFS = 0x00946464;
constexpr auto MOVECLIENTTOINTERMISSION_OFS = 0x00087A14;
constexpr auto STOPFOLLOWING_OFS = 0x0012B354;
constexpr auto LEVEL_OFS = 0x0068A3A0;
constexpr auto INTERMISSION_ORIGIN_OFS = 0x234C;
constexpr auto INTERMISSION_ANGLE_OFS = 0x2358;
constexpr auto TEAMSCORES_OFS = 0x2C;

static sharedEntity_t *(*G_SoundTempEntity)(vec3_t origin, int event, int channel) = nullptr;
static void (*G_MuteSound)(int, int) = nullptr;
static void (*G_FreeEntity)(sharedEntity_t*) = nullptr;
static void (*_BG_AddPredictableEventToPlayerstate)(int, int, playerState_t*) = nullptr;
static pmove_t **BG_PM = nullptr;
static void (*StopFollowing)(sharedEntity_t*) = nullptr;
static vec3_t *level_intermission_origin = nullptr;
static vec3_t *level_intermission_angle = nullptr;
static int (*level_teamScores)[TEAM_NUM_TEAMS] = nullptr;

static void G_InitGentity(sharedEntity_t *ent) {
	jampog::Entity(ent).set_inuse(true);
	ent->classname = "noclass";
	ent->s.number = SV_NumForGentity(ent);
	ent->r.ownerNum = ENTITYNUM_NONE;
	ent->s.modelGhoul2 = 0;
	ent->s.clientNum = ENTITYNUM_NONE;
	ent->s.otherEntityNum = ENTITYNUM_NONE;
	// must be networked back to 0 if still ENTITYNUM_NONE
	ent->s.otherEntityNum2 = MAX_GENTITIES;
	ent->s.trickedentindex = MAX_GENTITIES;
	ent->s.trickedentindex2 = ENTITYNUM_NONE;
	ent->s.owner = ENTITYNUM_NONE;
	ent->r.singleClient = ENTITYNUM_NONE;
	ICARUS_FreeEnt(ent);
}

static void G_Sound(sharedEntity_t *ent, int channel, int soundIndex) {
	auto te = G_SoundTempEntity(ent->r.currentOrigin, EV_GENERAL_SOUND, channel);
	te->s.eventParm = soundIndex;
	te->s.saberEntityNum = channel;
	te->s.otherEntityNum = SV_NumForGentity(flatten(ent));

	if (ent->playerState
	    && 0 <= SV_NumForGentity(ent) && SV_NumForGentity(ent) < MAX_CLIENTS
	    && channel > TRACK_CHANNEL_NONE)
	{ //let the client remember the index of the player entity so he can kill the most recent sound on request
		if (jampog::Entity(ent->playerState->fd.killSoundEntIndex[channel-50]).inuse()
		    && ent->playerState->fd.killSoundEntIndex[channel-50] > MAX_CLIENTS)
		{
			G_MuteSound(ent->playerState->fd.killSoundEntIndex[channel-50], CHAN_VOICE);
			if (ent->playerState->fd.killSoundEntIndex[channel-50] > MAX_CLIENTS
			    && jampog::Entity(ent->playerState->fd.killSoundEntIndex[channel-50]).inuse())
			{
				G_FreeEntity(SV_GentityNum(ent->playerState->fd.killSoundEntIndex[channel-50]));
			}
			ent->playerState->fd.killSoundEntIndex[channel-50] = 0;
		}

		ent->playerState->fd.killSoundEntIndex[channel-50] = te->s.number;
		te->s.eFlags = EF_SOUNDTRACKER;
		te->r.svFlags |= SVF_BROADCAST;
		te->s.trickedentindex = ent->s.number;
	}
}

static void print_stats_team(client_t *this_cl, int team) {
	if (team == TEAM_BLUE) {
		console::writeln(this_cl, "team ^4BLUE^7");
	} else if (team == TEAM_RED) {
		console::writeln(this_cl, "team ^1RED^7");
	}
	console::writeln(this_cl, "^3%-36s %-10s %-10s %-8s %-8s^7", "name", "damage", "accuracy", "kills", "deaths");
	for (auto i = 0; i < sv_maxclients->integer; i++) {
		auto cl = svs.clients + i;
		jampog::Entity e(cl);
		if (cl->state != CS_ACTIVE) continue;
		if (e.client().team() == team) {
			console::writeln(this_cl,
				va("%s%d%s", "%-", color_diff(e.client().name()) + 36, "s^7 %-10d %-10d %-8d %-8d"),
				e.client().name(),
				cl->stats.damage(),
				cl->stats.accuracy(),
				cl->stats.kills(),
				cl->stats.deaths()
			);
		}
	}
}

static void print_stats(sharedEntity_t *ent) {
	if (Cvar_VariableIntegerValue("g_gametype") != GT_TEAM) return;
	auto this_cl = svs.clients + SV_NumForGentity(ent);
	console::writeln(this_cl, "^5** stats **^7");
	if ((*level_teamScores)[TEAM_BLUE] > (*level_teamScores)[TEAM_RED]) {
		console::writeln(this_cl, "^4BLUE^7 wins!\n");
		print_stats_team(this_cl, TEAM_BLUE);
		console::writeln(this_cl, "");
		print_stats_team(this_cl, TEAM_RED);
	} else if ((*level_teamScores)[TEAM_RED] > (*level_teamScores)[TEAM_BLUE]) {
		console::writeln(this_cl, "^1RED^7 wins!\n");
		print_stats_team(this_cl, TEAM_RED);
		console::writeln(this_cl, "");
		print_stats_team(this_cl, TEAM_BLUE);
	} else {
		console::writeln(this_cl, "teams are ^2TIED^7");
		print_stats_team(this_cl, TEAM_RED);
		console::writeln(this_cl, "");
		print_stats_team(this_cl, TEAM_BLUE);
	}
	console::writeln(this_cl, "");
}

static void MoveClientToIntermission(sharedEntity_t *ent) {
	auto cl = jampog::Entity(ent).client();

	// take out of follow mode if needed
	if (cl.spectator_state() == 2/*SPECTATOR_FOLLOW*/) {
		StopFollowing(ent);
	}

	// move to the spot
	VectorCopy(*level_intermission_origin, ent->s.origin);
	VectorCopy(*level_intermission_origin, ent->playerState->origin);
	VectorCopy(*level_intermission_angle, ent->playerState->viewangles);
	ent->playerState->pm_type = PM_INTERMISSION;

	// clean up powerup info
	memset(ent->playerState->powerups, 0, sizeof(ent->playerState->powerups));

	ent->playerState->rocketLockIndex = ENTITYNUM_NONE;
	ent->playerState->rocketLockTime = 0;

	ent->playerState->eFlags = 0;
	ent->s.eFlags = 0;
	ent->playerState->eFlags2 = 0;
	ent->s.eFlags2 = 0;
	ent->s.eType = ET_GENERAL;
	ent->s.modelindex = 0;
	ent->s.loopSound = 0;
	ent->s.loopIsSoundset = qfalse;
	ent->s.event = 0;
	ent->r.contents = 0;

	print_stats(ent);
}

static void PM_AddEvent(int newEvent) {
	auto ps = (*BG_PM)->ps;
	if (newEvent == EV_SABER_ATTACK) {
		svs.clients[ps->clientNum].stats.add_shot();
	}
	_BG_AddPredictableEventToPlayerstate(newEvent, 0, ps);
}

namespace jampog {
	void init(const vm_t * const vm) {
		Com_Printf("initializing jampog\n");
		const uintptr_t base = dladdress(vm->dllHandle);
		set_base((void*)base);
		Com_Printf("patching BG_LegalizedForcePowers\n");
		patch_forcepowers(base);
		Com_Printf("patching SetTeam\n");
		patch_setteam(base);
		Com_Printf("patching ClientCleanName\n");
		patch_clean_name(base);
		Com_Printf("patching Cmd_EngageDuel_f\n");
		patch_engage_duel(base);
		Com_Printf("patching CheatsOk\n");
		detour((void*)(base + 0x00129B94), (void*)cheats_okay);
		#if 0
		Com_Printf("unprotect g_entities[]\n");
		unprotect(Entity::start(), Entity::array_size());
		Com_Printf("unprotect g_clients[]\n");
		unprotect(Client::start(), Client::array_size());
		#endif
		Com_Printf("patching bot_honorableduelacceptance\n");
		// remove cheat protection
		patch_byte((unsigned char*)(base + 0xE1364), 0);
		Com_Printf("patching G_InitGentity\n");
		detour((void*)(base + 0x0016D984), (void*)G_InitGentity);
		Com_Printf("patching G_Sound\n");
		G_SoundTempEntity = (decltype(G_SoundTempEntity))(base + G_SOUNDTEMPENTITY_OFS);
		G_MuteSound = (decltype(G_MuteSound))(base + G_MUTESOUND_OFS);
		G_FreeEntity = (decltype(G_FreeEntity))(base + G_FREEENTITY_OFS);
		detour((void*)(base + 0x0016E824), (void*)G_Sound);
		Com_Printf("patching PM_AddEvent\n");
		detour((void*)(base + 0x00101A34), (void*)PM_AddEvent);
		_BG_AddPredictableEventToPlayerstate = (decltype(_BG_AddPredictableEventToPlayerstate))(base + 0x000EBBE4);
		BG_PM = (decltype(BG_PM))(base + PMOVE_OFS);
		Com_Printf("patching MoveClientToIntermission\n");
		detour((void*)(base + MOVECLIENTTOINTERMISSION_OFS), (void*)MoveClientToIntermission);
		StopFollowing = (decltype(StopFollowing))(base + STOPFOLLOWING_OFS);
		level_intermission_angle = (decltype(level_intermission_angle))(base + LEVEL_OFS + INTERMISSION_ANGLE_OFS);
		level_intermission_origin = (decltype(level_intermission_origin))(base + LEVEL_OFS + INTERMISSION_ORIGIN_OFS);
		level_teamScores = (decltype(level_teamScores))(base + LEVEL_OFS + TEAMSCORES_OFS);
		patch_damage_hooks(base);
	}
}