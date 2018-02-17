#pragma once

namespace jampog {
	void detour(void * const before, const void * const after);
	void patch_byte(unsigned char *byte, unsigned char value);
	void patch_word(unsigned int *word, unsigned int value);
	void patch_str(void *dest, const char *str);
}
