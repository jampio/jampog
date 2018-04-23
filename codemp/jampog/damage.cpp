#include <server/server.h>
#include "damage.h"
#include "Entity.h"

constexpr auto CHECKARMOR_OFS = 0x00135244;
constexpr auto PLAYERDIE_OFS = 0x00133B44;
constexpr auto G_LOGWEAPONDAMAGE_OFS = 0x001413C4;

static int (*CheckArmor)(sharedEntity_t *ent, int damage, int dflags) = nullptr;
static void (*player_die)(sharedEntity_t *self, sharedEntity_t *inflictor, sharedEntity_t *attacker, int damage, int mod) = nullptr;
static int (*G_WeaponLogDamage)[MAX_CLIENTS][MOD_MAX] = nullptr;
static qboolean (*G_WeaponLogClientTouch)[MAX_CLIENTS] = nullptr;

static int CheckArmorProxy(sharedEntity_t *target, int damage, int dflags) {
	register sharedEntity_t *esi asm("esi");
	jampog::Entity attacker(esi);
	auto amt = CheckArmor(target, damage, dflags);
	if (attacker.is_player() && attacker.gent_ptr() != target) {
		svs.clients[attacker.number()].stats.add_damage(amt);
	}
	return amt;
}

static void PlayerDieProxy(sharedEntity_t *self, sharedEntity_t *inflictor, sharedEntity_t *attacker, int damage, int mod) {
	if (attacker != nullptr
	    && self != attacker
	    && jampog::Entity(attacker).is_player()) {
		auto health = self->playerState->stats[STAT_HEALTH];
		auto dmg = damage + health;
		svs.clients[SV_NumForGentity(attacker)].stats.add_damage(dmg);
	}
	player_die(self, inflictor, attacker, damage, mod);
}

static void G_LogWeaponDamage(int client, int mod, int amount) {
	if (client >= MAX_CLIENTS) return;
	(*G_WeaponLogDamage)[client][mod] += amount;
	(*G_WeaponLogClientTouch)[client] = qtrue;

	svs.clients[client].stats.add_damage(amount);
}

void jampog::patch_damage_hooks(uintptr_t base) {
	Com_Printf("patching G_LogWeaponDamage\n");
	detour((void*)(base + G_LOGWEAPONDAMAGE_OFS), (void*)G_LogWeaponDamage);
	G_WeaponLogDamage = (decltype(G_WeaponLogDamage))(base + 0x0099B640);
	G_WeaponLogClientTouch = (decltype(G_WeaponLogClientTouch))(base + 0x0099CBC0);
	Com_Printf("patching G_Damage\n");
	CheckArmor = (decltype(CheckArmor))(base + CHECKARMOR_OFS);
	detour_call((void*)(base + 0x00138F35), (void*)CheckArmorProxy);
	Com_Printf("patching player_die\n");
	player_die = (decltype(player_die))(base + PLAYERDIE_OFS);
	patch_word((unsigned int*)(base + 0x001284A6 + 6), (unsigned int)PlayerDieProxy);
}