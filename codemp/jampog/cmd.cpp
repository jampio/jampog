#include "cmd.h"
#include "server/server.h"
#include <memory>
#include <utility>
#include <functional>

cvar_t *Cvar_FindVar(const char *var_name);
static cvar_t *admin_password = nullptr;

namespace console {
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

static void players(client_t *cl) {
	for (int i = 0; i < sv_maxclients->integer; i++) {
		client_t *c = &svs.clients[i];
		if (c->state == CS_ACTIVE) {
			console::writeln(cl, "%d - %s", c->gentity->s.number, c->name);
		}
	}
}

static void login(client_t *cl) {
	if (cl->admin.logged_in) {
		console::writeln(cl, "You are logged in.");
		return;
	}
	if (!strcmp(admin_password->string, "") || strcmp(admin_password->string, Cmd_Argv(1))) {
		console::writeln(cl, "Invalid password.");
	} else {
		cl->admin.logged_in = true;
		console::writeln(cl, "You are logged in.");
	}
}

static void amfraglimit(client_t *cl) {
	if (Cmd_Argc() != 2) {
		console::writeln(cl, "amfraglimit <fraglimit>");
		return;
	}
	int fraglimit = atoi(Cmd_Argv(1));
	if (fraglimit < 0 || fraglimit > 100) {
		fraglimit = 0;
	}
	Cvar_Set("fraglimit", va("%d", fraglimit));
}

static void ammap(client_t *cl) {
	if (Cmd_Argc() < 2) {
		console::writeln(cl, "ammap <map> [g_gametype = current]");
		return;
	}
	if (FS_ReadFile(va("maps/%s.bsp", Cmd_Argv(1)), nullptr) == -1) {
		console::writeln(cl, "Can't find map %s", Cmd_Argv(1));
		return;
	}
	if (Cmd_Argc() == 3) {
		const char * const gt = Cmd_Argv(2);
		if (strlen(gt) == 1) {
			char byte = gt[0] - 48;
			if (byte >= GT_FFA && byte < GT_MAX_GAME_TYPE) {
				Cvar_Set("g_gametype", gt);
			} else {
				console::writeln(cl, "Invalid gametype");
				return;
			}
		} else {
			gametype_t new_gt = GT_MAX_GAME_TYPE;
			if (!strcmp(gt, "ffa")) {
				new_gt = GT_FFA;
			} else if (!strcmp(gt, "holocron")) {
				new_gt = GT_HOLOCRON;
			} else if (!strcmp(gt, "jedimaster")) {
				new_gt = GT_JEDIMASTER;
			} else if (!strcmp(gt, "duel")) {
				new_gt = GT_DUEL;
			} else if (!strcmp(gt, "powerduel")) {
				new_gt = GT_POWERDUEL;
			} else if (!strcmp(gt, "singleplayer")) {
				new_gt = GT_SINGLE_PLAYER;
			} else if (!strcmp(gt, "tffa") || !strcmp(gt, "tdm")) {
				new_gt = GT_TEAM;
			} else if (!strcmp(gt, "siege")) {
				new_gt = GT_SIEGE;
			} else if (!strcmp(gt, "ctf")) {
				new_gt = GT_CTF;
			} else if (!strcmp(gt, "cty")) {
				new_gt = GT_CTY;
			}
			if (new_gt == GT_MAX_GAME_TYPE) {
				console::writeln(cl, "Invalid gametype");
				return;
			}
			if (new_gt == GT_DUEL || new_gt == GT_POWERDUEL) {
				Cvar_Set("fraglimit", "1");
			} else if (new_gt == GT_FFA) {
				Cvar_Set("fraglimit", Cvar_FindVar("fraglimit")->resetString);
			}
			Cvar_Set("g_gametype", va("%d", new_gt));
		}
	}
	console::exec("map %s", Cmd_Argv(1));
}

static void amtimelimit(client_t *cl) {
	if (Cmd_Argc() != 2) {
		console::writeln(cl, "amtimelimit <timelimit>");
		return;
	}
	int timelimit = atoi(Cmd_Argv(1));
	if (timelimit < 0 || timelimit > 100) {
		timelimit = 0;
	}
	Cvar_Set("timelimit", va("%d", timelimit));
}

struct Command {
	const char *name;
	std::function<void(client_t*)> func;
	const char *desc;
};

static void info(client_t*);

static Command cmds[] = {
	{"info", info, "show this"},
	{"players", players, "show a list of players"},
	{"login", login, "login to admin"},
};

static Command admin_cmds[] = {
	{"amfraglimit", amfraglimit, "change fraglimit"},
	{"ammap", ammap, "change map, <mapname> <optional gametype> ex. ammap mp/duel1 duel"},
	{"amtimelimit", amtimelimit, "change timelimit"},
};

static struct {
	const char *alias;
	const char *orig;
} aliases[] = {
	{"amlogin", "login"},
	{"showplayerid", "players"},
	{"aminfo", "info"}
};

static const char *unalias(const char *arg) {
	for (auto &it: aliases) {
		if (!strcmp(arg, it.alias)) {
			return it.orig;
		}
	}
	return arg;
}

static const char *pad(const char *text, const int PAD = 24) {
	static char buffer[1024];
	const size_t len = strlen(text);
	if (len >= sizeof(buffer)) {
		fprintf(stderr, "len is larger than buffer\n");
		exit(EXIT_FAILURE);
	}
	const size_t p = PAD - len;
	if (p < 0) {
		fprintf(stderr, "NEGATIVE PAD\n");
		exit(EXIT_FAILURE);
	}
	size_t i = 0;
	for (; i < p; i++) {
		buffer[i] = ' ';
	}
	buffer[i] = 0;
	return buffer;
}

constexpr auto INFO = R"INFO(^5jampog:^7 An engine that runtime patches basejka.
sv_pure:    ^3%d^7
sv_fps:     ^3%s^7%s(your snaps: ^3%d^7)
sv_maxRate: ^3%s^7%s(your rate:  ^3%d^7)

commands:)INFO";

static void info(client_t *cl) {
	{
		char fps[16];
		Com_sprintf(fps, sizeof(fps), "%d", sv_fps->integer);
		char maxRate[32];
		Com_sprintf(maxRate, sizeof(maxRate), "%d", sv_maxRate->integer);
		char pad1[64];
		Q_strncpyz(pad1, pad(fps, 8), sizeof(pad1));
		char pad2[64];
		Q_strncpyz(pad2, pad(maxRate, 8), sizeof(pad2));
		console::writeln(cl, INFO,
			sv_pure->integer,
			fps, pad1, 1000 / cl->snapshotMsec,
			maxRate, pad2, cl->rate
		);
	}
	for (auto &cmd: cmds) {
		console::writeln(cl, "^5%s^7%s%s", cmd.name, pad(cmd.name), cmd.desc);
	}
	if (cl->admin.logged_in) {
		for (auto &cmd: admin_cmds) {
			console::writeln(cl, "^5%s^7%s%s", cmd.name, pad(cmd.name), cmd.desc);
		}
	}
}

bool jampog::command(client_t *cl) {
	const char * const arg0 = unalias(Cmd_Argv(0));
	if (!strcmp(arg0, "gc")) {
		return true;
	}
	for (auto &cmd: cmds) {
		if (!strcmp(arg0, cmd.name)) {
			cmd.func(cl);
			return true;
		}
	}
	for (auto &cmd: admin_cmds) {
		if (!strcmp(arg0, cmd.name)) {
			if (cl->admin.logged_in) {
				cmd.func(cl);
			} else {
				console::writeln(cl, "You are not logged in.");
			}
			return true;
		}
	}
	return false;
}

void jampog::command_init() {
	admin_password = Cvar_Get("admin_password", "", 0);
}
