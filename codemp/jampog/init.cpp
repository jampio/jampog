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

qboolean cheats_okay(void *ptr);

constexpr auto G_SOUNDTEMPENTITY_OFS = 0x0016E224;
constexpr auto G_MUTESOUND_OFS = 0x0016E7A4;
constexpr auto G_FREEENTITY_OFS = 0x0016DE44;
constexpr auto PMOVE_OFS = 0x00946464;
constexpr auto G_LOGWEAPONDAMAGE_OFS = 0x001413C4;

static sharedEntity_t *(*G_SoundTempEntity)(vec3_t origin, int event, int channel) = nullptr;
static void (*G_MuteSound)(int, int) = nullptr;
static void (*G_FreeEntity)(sharedEntity_t*) = nullptr;
static void (*_BG_AddPredictableEventToPlayerstate)(int, int, playerState_t*) = nullptr;
static pmove_t **BG_PM = nullptr;
int (*G_WeaponLogDamage)[MAX_CLIENTS][MOD_MAX] = nullptr;
qboolean (*G_WeaponLogClientTouch)[MAX_CLIENTS] = nullptr;

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

#if 0
static void patch_fire_weapon(const uintptr_t base) {
	using jampog::patch_byte;
	// effectively allows accuracy_shots for any weapons
	// will give it dummy values for it to fail to
	unsigned char dummy = -10;//0x99;
	patch_byte((unsigned char*)(base + 0x0017E164), dummy);
	patch_byte((unsigned char*)(base + 0x0017E169), dummy);
	patch_byte((unsigned char*)(base + 0x0017E16E), dummy);
}
#endif

static void PM_AddEvent(int newEvent) {
	auto ps = (*BG_PM)->ps;
	if (newEvent == EV_SABER_ATTACK) {
		svs.clients[ps->clientNum].stats.add_shot();
	}
	_BG_AddPredictableEventToPlayerstate(newEvent, 0, ps);
}

// This currently will not catch SHIELD_HITS
// for that I added an extra check "check_hits()"
// which is called during client think
// which checks changes in PERS_HITS
#if 0
static void G_LogWeaponDamage(int client, int mod, int amount) {
	if (client >= MAX_CLIENTS) return;
	(*G_WeaponLogDamage)[client][mod] += amount;
	(*G_WeaponLogClientTouch)[client] = qtrue;

	if (mod == MOD_SABER) {
		svs.clients[client].stats.add_hit();
	}
}
#endif

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
		//Com_Printf("patching FireWeapon\n");
		//patch_fire_weapon(base);
		Com_Printf("patching PM_AddEvent\n");
		detour((void*)(base + 0x00101A34), (void*)PM_AddEvent);
		_BG_AddPredictableEventToPlayerstate = (decltype(_BG_AddPredictableEventToPlayerstate))(base + 0x000EBBE4);
		BG_PM = (decltype(BG_PM))(base + PMOVE_OFS);
		#if 0
		Com_Printf("patching G_LogWeaponDamage\n");
		detour((void*)(base + G_LOGWEAPONDAMAGE_OFS), (void*)G_LogWeaponDamage);
		G_WeaponLogDamage = (decltype(G_WeaponLogDamage))(base + 0x0099B640);
		G_WeaponLogClientTouch = (decltype(G_WeaponLogClientTouch))(base + 0x0099CBC0);
		#endif
	}
}