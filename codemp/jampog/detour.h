#pragma once

#include <cstdlib>
#include <cstdint>

namespace jampog {
	uintptr_t dladdress(void * const handle);
	void unprotect(const void * const addr, const size_t len);
	void detour(void * const before, const void * const after);
	void patch_byte(unsigned char *byte, unsigned char value);
	void patch_word(unsigned int *word, unsigned int value);
	void patch_str(void *dest, const char *str);
}
