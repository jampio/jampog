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
	static constexpr auto LOCALANIMINDEX_OFS = 0x220;
	static constexpr auto PARENT_OFS = 0x40C;
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
	Entity(int index)
		: Entity(SV_GentityNum(index))
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
	entityState_t& s() const {
		return ((sharedEntity_t*)base)->s;
	}
	playerState_t& ps() const {
		playerState_t *ps = ((sharedEntity_t*)base)->playerState;
		if (ps == nullptr) throw "NULL PLAYERSTATE";
		return *ps;
	}
	entityShared_t& r() const {
		return ((sharedEntity_t*)base)->r;
	}
	sharedEntity_t& gent() const {
		return *(sharedEntity_t*)base;
	}
	sharedEntity_t *gent_ptr() const {
		return (sharedEntity_t*)base;
	}
	int number() const {
		return SV_NumForGentity(gent_ptr());
	}
	bool inuse() const {
		return *(qboolean*)(base + INUSE_OFS) == qtrue;
	}
	void set_inuse(bool value) {
		*(qboolean*)(base + INUSE_OFS) = value ? qtrue : qfalse;
	}
	// returns false if player is spectating
	bool is_player() const {
		return s().eType == ET_PLAYER;
	}
	bool is_npc() const {
		return s().eType == ET_NPC;
	}
	int health() const {
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
	void teleport(float x, float y, float z) {
		vec3_t orig = {x, y, z};
		teleport(orig);
	}
	void teleport(const Entity& To) {
		float x = To.origin()[0];
		float y = To.origin()[1];
		float z = To.origin()[2];
		if (To.is_player() || To.is_npc()) {
			z += 120.0f;
		}
		teleport(x, y, z);
	}
	vec3_t& origin() const {
		if (0 <= number() && number() < MAX_CLIENTS) {
			return ps().origin;
		} else {
			return r().currentOrigin;
		}
	}
	int local_anim_index() const {
		return *(int*)(base + LOCALANIMINDEX_OFS);
	}
	sharedEntity_t *parent_ptr() const {
		return *(sharedEntity_t**)(base + PARENT_OFS);
	}
	Entity parent() const {
		return parent_ptr();
	}
	void set_parent(sharedEntity_t *ent) {
		*(sharedEntity_t**)(base + PARENT_OFS) = ent;
	}
};
}
