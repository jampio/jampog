#pragma once

#include <cstdint>

namespace jampog {
	void set_base(void *base);
	void set_base(uintptr_t base);
	void *base_ptr();
	uintptr_t base_addr();
}