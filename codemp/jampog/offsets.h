#pragma once

#include "server/server.h"
#include <cstdint>
#include "structs/clientPersistant_t.h"

static inline void *gentities(uintptr_t base) {
	return (void*)(base + 0x006CE620);
}

static inline int client_num(uintptr_t base, void *ent) {
	return ((uintptr_t)ent - (uintptr_t)gentities(base)) / 1516;
}

static inline entityState_t *ent_s(void *ent) {
	return (entityState_t *)ent;
}

static inline playerState_t *client_ps(void *client) {
	return (playerState_t*)client;
}

static inline clientPersistant_t *client_pers(void *client) {
	return (clientPersistant_t*)((uintptr_t)client + 1552);
}

static inline void client_set_invulnerableTimer(void *client, int value) {
	*(int*)((uintptr_t)client + 6328) = value;
}

static inline saberInfo_t *client_saber(void *client) {
	return (saberInfo_t*)((uintptr_t)client + 1992);
}

static inline void *gentity_for_num(uintptr_t base, int num) {
	constexpr auto gent_size = 1516;
	return (void*)((uintptr_t)gentities(base) + (gent_size * num));
}

static inline int ent_health(void *ent) {
	return *(int*)((uintptr_t)ent + 1232);
}

static inline void ent_set_health(void *ent, int health) {
	*(int*)((uintptr_t)ent + 1232) = health;
}

static inline void *ent_client(void *ent) {
	return *(void**)((uintptr_t)ent + 864);
}

static inline qboolean ent_inuse(void *ent) {
	return *(qboolean*)((uintptr_t)ent + 880);
}
