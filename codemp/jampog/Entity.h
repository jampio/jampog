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
	static constexpr size_t size() {
		return SIZE;
	}
	static constexpr size_t array_size() {
		return SIZE * 1024;
	}
	static void *start() {
		return (void*)((uintptr_t)get_base() + GENTITY_OFS);
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
};
}
