#include "util.h"

bool jampog::str::any_chars(const char *chars, const char *subject) {
	auto len = strlen(subject);
	for (decltype(len) i = 0; i < len; i++) {
		if (strchr(chars, subject[i])) return true;
	}
	return false;
}

bool jampog::str::empty(const char *str) {
	if (!str || !str[0]) return true;
	return false;
}