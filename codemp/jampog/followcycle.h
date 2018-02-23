#pragma once

#include <cstdint>

namespace jampog {
	void patch_followcycle(uintptr_t base);
	void patch_setteam(uintptr_t base);
}
