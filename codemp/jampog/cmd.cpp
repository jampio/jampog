#include "cmd.h"
#include "server/server.h"
#include <memory>
#include <utility>
#include <functional>

static cvar_t *admin_password = nullptr;

constexpr auto INFO = R"INFO(
^5** jampog **^7
An engine that runtime patches basejka.

sv_pure: %d
sv_fps: %d

commands:
info        show this
players     display a list of players
login       login to admin
)INFO";

static void info(client_t *cl) {
	// va is safe to call up to 2 times
	SV_SendServerCommand(cl, va("print \"%s\n\"", va(INFO, sv_pure->integer, sv_fps->integer)));
}

static void gc(client_t *cl) {}

static void players(client_t *cl) {
	for (int i = 0; i < sv_maxclients->integer; i++) {
		client_t *c = &svs.clients[i];
		if (c->state == CS_ACTIVE) {
			SV_SendServerCommand(cl, va("print \"%d - %s\n\"", c->gentity->s.number, c->name));
		}
	}
}

static void login(client_t *cl) {
	if (cl->admin.logged_in) {
		SV_SendServerCommand(cl, "print \"You are logged in.\n\"");
		return;
	}
	if (!strcmp(admin_password->string, "") || strcmp(admin_password->string, Cmd_Argv(1))) {
		SV_SendServerCommand(cl, "print \"Invalid password.\n\"");
	} else {
		cl->admin.logged_in = true;
		SV_SendServerCommand(cl, "print \"You are logged in.\n\"");
	}
}

static void ammap(client_t *cl) {
	if (FS_ReadFile(va("maps/%s.bsp", Cmd_Argv(1)), nullptr) == -1) {
		SV_SendServerCommand(cl, "print \"Can't find map %s\n\"", Cmd_Argv(1));
		return;
	}
	Cbuf_ExecuteText(EXEC_NOW, va("map %s\n", Cmd_Argv(1)));
}

template <typename Proc>
static auto admin(Proc &&proc) {
	return [proc = std::forward<Proc>(proc)](client_t *cl) {
		if (!cl->admin.logged_in) {
			SV_SendServerCommand(cl, "print \"You are not logged in.\n\"");
			return;
		}
		proc(cl);
	};
}

struct Command {
	const char	*name;
	std::function<void(client_t *cl)> func;
};

static Command cmds[] = {
	{"gc", gc},
	{"info", info},
	{"aminfo", info},
	{"players", players},
	{"login", login},
	{"amlogin", login},
	{"ammap", admin(ammap)}
};

bool jampog::command(client_t *cl) {
	for (auto &cmd: cmds) {
		if (!strcmp(Cmd_Argv(0), cmd.name)) {
			cmd.func(cl);
			return true;
		}
	}
	return false;
}

void jampog::command_init() {
	admin_password = Cvar_Get("admin_password", "", 0);
}
