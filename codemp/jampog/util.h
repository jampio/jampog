#pragma once

#include "server/server.h"

namespace jampog::console {
	template <typename... Args>
	static void writeln(client_t *cl, const char *fmt, Args... args) {
		char buffer[2048] = {0};
		char format[2048] = {0};
		Com_sprintf(format, sizeof(format), fmt, args...);
		Q_strcat(buffer, sizeof(buffer), "print \"");
		Q_strcat(buffer, sizeof(buffer), format);
		Q_strcat(buffer, sizeof(buffer), "\n\"");
		SV_SendServerCommand(cl, buffer);
	}
	template <typename... Args>
	static void exec(const char *fmt, Args... args) {
		char buffer[1024] = {0};
		Com_sprintf(buffer, sizeof(buffer) - 2, fmt, args...);
		Q_strcat(buffer, sizeof(buffer), "\n");
		Cbuf_ExecuteText(EXEC_APPEND, buffer);
	}
}

namespace jampog::str {
	template <size_t N>
	static void cpy(char (&buffer)[N], const char *src) {
		Q_strncpyz(buffer, src, N);
	}
	bool any_chars(const char *chars, const char *subject);
	bool empty(const char *str);
	bool equals(const char *a, const char *b);
}

namespace jampog {
	template <size_t N>
	static int color_diff(const char (&buf)[N]) {
		char nocolor[N];
		Q_strncpyz(nocolor, buf, N);
		Q_StripColor(nocolor);
		return strlen(buf) - strlen(nocolor);
	}
}