#pragma once

#include "base.h"
#include "Client.h"

namespace jampog {
class Entity {
private:
	static constexpr auto CLIENT_OFS = 864;
	static constexpr auto HEALTH_OFS = 1232;
	static constexpr size_t SIZE = 1516;
	static constexpr auto GENTITY_OFS = 0x006CE620;
	static constexpr auto INUSE_OFS = 880;
	uintptr_t base;
public:
	Entity() = delete;
	Entity(uintptr_t base)
		: base(base)
	{}
	Entity(void *ptr)
		: Entity((uintptr_t)ptr)
	{}
	Entity(client_t *cl)
		: Entity(cl->gentity)
	{}
	Entity(sharedEntity_t *ent)
		: Entity((void*)ent)
	{}
	static constexpr size_t size() {
		return SIZE;
	}
	static constexpr size_t array_size() {
		return SIZE * 1024;
	}
	static void *start() {
		return (void*)(base_addr() + GENTITY_OFS);
	}
	entityState_t& s() {
		return ((sharedEntity_t*)base)->s;
	}
	playerState_t& ps() {
		playerState_t *ps = ((sharedEntity_t*)base)->playerState;
		if (ps == nullptr) throw "NULL PLAYERSTATE";
		return *ps;
	}
	entityShared_t& r() {
		return ((sharedEntity_t*)base)->r;
	}
	bool inuse() {
		return *(qboolean*)(base + INUSE_OFS) == qtrue;
	}
	// returns false if player is spectating
	bool is_player() {
		return s().eType == ET_PLAYER;
	}
	int health() {
		return *(int*)(base + HEALTH_OFS);
	}
	void set_health(int value) {
		*(int*)(base + HEALTH_OFS) = value;
	}
	Client client() {
		return Client(*(uintptr_t*)(base + CLIENT_OFS));
	}
	void teleport(const vec3_t origin) {
		void (*TeleportPlayer)(void *ent, const vec3_t orig, vec3_t angles);
		TeleportPlayer = (decltype(TeleportPlayer))(base_addr() + 0x00146324);
		TeleportPlayer((void*)base, origin, s().angles);
	}
	vec3_t& origin() {
		auto ent = (sharedEntity_t*)base;
		if (ent->s.number >= 0 && ent->s.number < MAX_CLIENTS) {
			return ps().origin;
		} else {
			return r().currentOrigin;
		}
	}
};
}
