#include "clientname.h"
#include <array>

// an ext is "^7 (^5XX^7)"
// name extensions require 11 bytes
constexpr auto NAME_EXT_SIZE = 11;
constexpr auto NAME_FMT = "%s^7 (^5%d^7)";
constexpr auto NAME_SIZE = MAX_NETNAME - NAME_EXT_SIZE;

template <size_t N>
static std::array<char, N> remove_color(const std::array<char, N>& buf) {
	std::array<char, N> no_color;
	Q_strncpyz(no_color.data(), buf.data(), N);
	Q_StripColor(no_color.data());
	return no_color;
}

template <size_t N>
static std::array<char, N> remove_color(const char (&buf)[N]) {
	std::array<char, N> no_color;
	Q_strncpyz(no_color.data(), buf, N);
	Q_StripColor(no_color.data());
	return no_color;
}

template <size_t N>
static bool name_equals(const std::array<char, N>& left, const char (&right)[N]) {
	return Q_stricmp(remove_color(left).data(), remove_color(right).data()) == 0;
}

template <size_t N>
static bool unique(client_t *ignore, const std::array<char, N>& name) {
	for (auto i = 0; i < sv_maxclients->integer; i++) {
		auto& cl = svs.clients[i];
		if (cl.state == CS_FREE) continue;
		if (ignore == &cl) continue;
		if (name_equals(name, cl.name)) return false;
	}
	return true;
}

template <size_t N>
static std::array<char, N> remove_ctrl(const char *buf) {
	std::array<char, N> res;
	size_t j = 0;
	for (size_t i = 0; buf[i]; i++) {
		switch (buf[i]) {
			case '\r':
			case ';':
			case '\n':
			case '\\':
				continue;
		}
		res[j++] = buf[i];
	}
	res[j] = 0;
	return res;
}

template <size_t N>
static std::array<char, N> remove_ctrl(const char (&buf)[N]) {
	return remove_ctrl<N>(buf + 0);
}

template <size_t N>
static std::array<char, N> remove_ctrl(const std::array<char, N>& buf) {
	return remove_ctrl<N>(buf.data());
}

template <size_t N>
static std::array<char, N> ltrim(const char *buf) {
	std::array<char, N> res;
	size_t i = 0;
	while (true) {
		if (buf[i] == ' ') i += 1;
		else if (Q_IsColorStringExt(buf+i) && buf[i+2] == ' ') i += 3;
		else if (Q_IsColorStringExt(buf+i) && Q_IsColorStringExt(buf+i+2)) i += 4;
		else break;
	}
	Q_strncpyz(res.data(), buf + i, N);
	return res;
}

template <size_t N>
static std::array<char, N> ltrim(const char (&buf)[N]) {
	return ltrim<N>(buf + 0);
}

template <size_t N>
static std::array<char, N> ltrim(const std::array<char, N>& buf) {
	return ltrim<N>(buf.data());
}

template <size_t N>
static std::array<char, N> rtrim(const char *buf) {
	std::array<char, N> res;
	size_t i = strlen(buf) - 1;
	while (i >= 0 && buf[i] == ' ') i--;
	Q_strncpyz(res.data(), buf, i+2);
	return res;
}

template <size_t N>
static std::array<char, N> rtrim(const char (&buf)[N]) {
	return rtrim<N>(buf + 0);
}

template <size_t N>
static std::array<char, N> rtrim(const std::array<char, N>& buf) {
	return rtrim<N>(buf.data());
}

template <size_t N>
static std::array<char, N> clean_name(const std::array<char, N>& buf) {
	auto A = remove_ctrl(buf);
	auto B = ltrim(A);
	auto C = rtrim(B);
	return C;
}

template <size_t N>
static std::array<char, N> make_index(const std::array<char, N>& new_name, int index) {
	char name[NAME_SIZE];
	Q_strncpyz(name, new_name.data(), sizeof(name));
	std::array<char, N> result;
	Com_sprintf(result.data(), N, NAME_FMT, name, index);
	return result;
}

template <size_t N>
void set_name(client_t *cl, const std::array<char, N>& name) {
	Q_strncpyz(cl->name, name.data(), sizeof(cl->name));
	Info_SetValueForKey(cl->userinfo, "name", name.data());
}

static std::array<char, MAX_NETNAME> get_userinfo_name(client_t *cl) {
	std::array<char, MAX_NETNAME> name;
	Q_strncpyz(name.data(), Info_ValueForKey(cl->userinfo, "name"), name.max_size());
	return name;
}

template <size_t N>
static std::array<char, N> empty_or(std::array<char, N> buf, const char *alt) {
	if (!strcmp(buf.data(), "")) {
		Q_strncpyz(buf.data(), alt, N);
	}
	return buf;
}

void jampog::check_client_name(client_t *cl) {
	auto name = empty_or(clean_name(get_userinfo_name(cl)), "Padawan");
	if (unique(cl, name)) {
		set_name(cl, name);
		return;
	}
	auto index = 2;
	auto indexed = make_index(name, index++);
	while (!unique(cl, indexed)) {
		indexed = make_index(name, index++);
	}
	set_name(cl, indexed);
}