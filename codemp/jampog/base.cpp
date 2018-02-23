#include <cstdlib>
#include <cstdio>

static void *_base = nullptr;

namespace jampog {

void set_base(void *base) {
	_base = base;
}

void *get_base() {
	if (_base == nullptr) {
		fprintf(stderr, "base not initialized\n");
		exit(EXIT_FAILURE);
	} else {
		return _base;
	}
}

}