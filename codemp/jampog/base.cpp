#include <cstdlib>
#include <cstdio>
#include <cstdint>

static void *_base = nullptr;

static void *get() {
	if (_base == nullptr) {
		fprintf(stderr, "base not initialized\n");
		exit(EXIT_FAILURE);
	} else {
		return _base;
	}
}

namespace jampog {
	void set_base(void *base) {
		_base = base;
	}
	void set_base(uintptr_t base) {
		set_base((void*)base);
	}
	void *base_ptr() {
		return get();
	}
	uintptr_t base_addr() {
		return (uintptr_t)get();
	}
}