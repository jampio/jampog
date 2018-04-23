#pragma once

#include "base.h"
#include "server/server.h"
#include <cstdint>
#include "structs/clientPersistant_t.h"
#include "detour.h"

namespace jampog {
class Client {
private:
	static constexpr auto NOCLIP_OFS = 6316;
	static constexpr auto PERS_OFS = 1552;
	static constexpr auto GCLIENT_OFS = 0x00695D00;
	static constexpr size_t SIZE = 7284;
	static constexpr auto INVULNERABLE_TIMER_OFS = 6328;
	static constexpr auto ACCURACY_SHOTS_OFS = 0x18F8;
	static constexpr auto ACCURACY_HITS_OFS = 0x18FC;
	static constexpr auto SPECTATOR_STATE_OFS = 0x6B4;
	static constexpr auto SESSION_TEAM_OFS = 0x6AC;
	uintptr_t base;
public:
	Client() = delete;
	Client(uintptr_t base)
		: base(base)
	{}
	Client(void *ptr)
		: Client((uintptr_t)ptr)
	{}
	static void *start() {
		return (void*)(base_addr() + GCLIENT_OFS);
	}
	static constexpr size_t size() {
		return SIZE;
	}
	static constexpr size_t array_size() {
		return SIZE * MAX_CLIENTS;
	}
	static Client from_number(int number) {
		return Client((uintptr_t)start() + size() * number);
	}
	clientPersistant_t *persistant() {
		return (clientPersistant_t*)(base + 1552);
	}
	uintptr_t address() {
		return base;
	}
	bool noclip() {
		return *(qboolean*)(base + NOCLIP_OFS) == qtrue;
	}
	void set_noclip(bool value) {
		*(qboolean*)(base + NOCLIP_OFS) = (value ? qtrue : qfalse);
	}
	char (&name())[MAX_NETNAME] {
		return persistant()->netname;
	}
	void set_invulnerable_timer(int value) {
		*(int*)(base + INVULNERABLE_TIMER_OFS) = value;
	}
	int accuracy_shots() const {
		return *(int*)(base + ACCURACY_SHOTS_OFS);
	}
	void set_accuracy_shots(int value) {
		*(int*)(base + ACCURACY_SHOTS_OFS) = value;
	}
	int accuracy_hits() const {
		return *(int*)(base + ACCURACY_HITS_OFS);
	}
	int spectator_state() const {
		return *(int*)(base + SPECTATOR_STATE_OFS);
	}
	int team() const {
		return *(int*)(base + SESSION_TEAM_OFS);
	}
};
}
