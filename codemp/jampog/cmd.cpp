#include "cmd.h"
#include "server/server.h"
#include <memory>
#include <utility>
#include <functional>
#include "Entity.h"
#include "init.h"
#include "offsets.h"
#include "base.h"
#include "structs/GClient.h"

cvar_t *Cvar_FindVar(const char *var_name);
const char *SV_GetStringEdString(char *refSection, char *refName);
qboolean SV_DelBanEntryFromList(int index);
void SV_WriteBans(void);
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

namespace str {
	template <size_t N>
	static void cpy(char (&buffer)[N], const char *src) {
		Q_strncpyz(buffer, src, N);
	}
}

static client_t *client_from_ent(void *ptr) {
	for (auto i = 0; i < sv_maxclients->integer; i++) {
		if (svs.clients[i].gentity == ptr) {
			return &svs.clients[i];
		}
	}
	return nullptr;
}

qboolean cheats_okay(void *ptr) {
	auto cl = client_from_ent(ptr);
	if (Cvar_VariableIntegerValue("sv_cheats") || cl->admin.logged_in) {
		return qtrue;
	} else {
		console::writeln(cl, "You are not logged in.");
		return qfalse;
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

static void where(client_t *cl) {
	if (Cmd_Argc() == 2 && !cl->admin.logged_in) {
		console::writeln(cl, "You are not logged in.");
		return;
	}
	jampog::Entity ent{
		Cmd_Argc() == 2 ? SV_GentityNum(atoi(Cmd_Argv(1))) : cl->gentity
	};
	if (!ent.inuse()) {
		console::writeln(cl, "Invalid entity number");
		return;
	}
	auto orig = ent.origin();
	console::writeln(cl, "%f %f %f", orig[0], orig[1], orig[2]);
}

static void amban(client_t *cl) {
	if (Cmd_Argc() != 2) {
		console::writeln(cl, "amban <player_id>");
		return;
	}
	int number = atoi(Cmd_Argv(1));
	if (number < 0 || number >= MAX_CLIENTS) {
		console::writeln(cl, "Client number out of range");
		return;
	}
	jampog::Entity ent{atoi(Cmd_Argv(1))};
	if (!ent.inuse()) {
		console::writeln(cl, "Invalid entity index");
		return;
	}
	auto kick_cl = client_from_ent((void*)ent.gent_ptr());
	if (kick_cl == nullptr) {
		console::writeln(cl, "null kick_cl");
		return;
	}
	netadr_t ip = cl->netchan.remoteAddress;
	int mask = 32;
	qboolean isexception = qfalse;
	if (ip.type != NA_IP) {
		console::writeln(cl, "Error: Can ban players connected via the internet only.");
		return;
	}
	// first check whether a conflicting ban exists that would supersede the new one.
	for (int index = 0; index < serverBansCount; index++ ) {
		auto curban = &serverBans[index];
		if (curban->subnet <= mask) {
			if ((curban->isexception || !isexception)
			    && NET_CompareBaseAdrMask(curban->ip, ip, curban->subnet)) {
					console::writeln(cl, "Ban supersedes an existing ban");
					return;
			}
		}
		if (curban->subnet >= mask) {
			if (!curban->isexception
			    && isexception
				&& NET_CompareBaseAdrMask(curban->ip, ip, mask)) {
				console::writeln(cl, "Ban supersdes an existing ban");
				return;
			}
		}
	}
	int index = 0;
	while (index < serverBansCount) {
		auto curban = &serverBans[index];
		if (curban->subnet > mask 
		    && (!curban->isexception || isexception)
			&& NET_CompareBaseAdrMask(curban->ip, ip, mask))
			SV_DelBanEntryFromList(index);
		else
			index++;
	}
	serverBans[serverBansCount].ip = ip;
	serverBans[serverBansCount].subnet = mask;
	serverBans[serverBansCount].isexception = isexception;
	serverBansCount++;
	SV_WriteBans();
	SV_DropClient(kick_cl, SV_GetStringEdString("MP_SVGAME","WAS_KICKED"));
	kick_cl->lastPacketTime = svs.time;
}

static void amduelfraglimit(client_t *cl) {
	if (Cmd_Argc() != 2) {
		console::writeln(cl, "amduelfraglimit <fraglimit>");
		return;
	}
	int fraglimit = atoi(Cmd_Argv(1));
	if (fraglimit < 0 || fraglimit > 100) {
		fraglimit = 0;
	}
	Cvar_Set("duel_fraglimit", va("%d", fraglimit));
}

static void amduelweapondisable(client_t *cl) {
	if (Cmd_Argc() != 2) {
		console::writeln(cl, "amduelweapondisable <number> | saberonly | sabermeleeonly");
		return;
	}
	int disable;
	if (!strcmp(Cmd_Argv(1), "saberonly")) {
		disable = 131063;
	} else if (!strcmp(Cmd_Argv(1), "sabermeleeonly")) {
		disable = 131059;
	} else {
		disable = atoi(Cmd_Argv(1));
	}
	if (disable < 0) {
		disable = 0;
	}
	Cvar_Set("g_duelWeaponDisable", va("%d", disable));
}

static void amforcepowerdisable(client_t *cl) {
	if (Cmd_Argc() != 2) {
		console::writeln(cl, "amforcepowerdisable <number> | nf");
		return;
	}
	int disable;
	if (!strcmp(Cmd_Argv(1), "nf")) {
		disable = 163837;
	} else {
		disable = atoi(Cmd_Argv(1));
	}
	if (disable < 0) {
		disable = 0;
	}
	Cvar_Set("g_forcePowerDisable", va("%d", disable));
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

static void amkick(client_t *cl) {
	if (Cmd_Argc() != 2) {
		console::writeln(cl, "amkick <player_id>");
		return;
	}
	int number = atoi(Cmd_Argv(1));
	if (number < 0 || number >= MAX_CLIENTS) {
		console::writeln(cl, "Client number out of range");
		return;
	}
	jampog::Entity ent{atoi(Cmd_Argv(1))};
	if (!ent.inuse()) {
		console::writeln(cl, "Invalid entity index");
		return;
	}
	auto kick_cl = client_from_ent((void*)ent.gent_ptr());
	if (kick_cl == nullptr) {
		console::writeln(cl, "null kick_cl");
		return;
	}
	SV_DropClient(kick_cl, SV_GetStringEdString("MP_SVGAME","WAS_KICKED"));
	kick_cl->lastPacketTime = svs.time;
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

static void ammaprestart(client_t *cl) {
	console::exec("map_restart");
}

#if 0
static void amnoclip(client_t *cl) {
	if (cl->admin.logged_in || Cvar_VariableIntegerValue("sv_cheats")) {
		auto player = jampog::Entity(cl).client();
		player.set_noclip(!player.noclip());
		if (player.noclip()) {
			console::writeln(cl, "noclip ON");
		} else {
			console::writeln(cl, "noclip OFF");
		}
	} else {
		console::writeln(cl, "Cheats are not enabled and not logged in");
	}
}
#endif

static void amtele(client_t *cl) {
	if (Cmd_Argc() == 1) {
		if (cl->telemark.is_marked()) {
			jampog::Entity(cl).teleport(cl->telemark.get());
		} else {
			console::writeln(cl, "Telemark not set");
		}
	} else if (Cmd_Argc() == 2) {
		jampog::Entity to{atoi(Cmd_Argv(1))};
		if (to.inuse()) {
			jampog::Entity(cl).teleport(to);
		} else {
			console::writeln(cl, "Invalid entity");
		}
	} else if (Cmd_Argc() == 3) {
		jampog::Entity a{atoi(Cmd_Argv(1))};
		jampog::Entity b{atoi(Cmd_Argv(2))};
		if (a.inuse() && b.inuse()) {
			a.teleport(b);
		} else {
			console::writeln(cl, "Invalid entity");
		}
	} else if (Cmd_Argc() == 4) {
		jampog::Entity(cl).teleport(
			float(atof(Cmd_Argv(1))),
			float(atof(Cmd_Argv(2))),
			float(atof(Cmd_Argv(3)))
		);
	}
}

static void amtelemark(client_t *cl) {
	auto origin = jampog::Entity(cl).origin();
	cl->telemark.set(origin[0], origin[1], origin[2]);
	console::writeln(cl, "telemark set");
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

static void amweapondisable(client_t *cl) {
	if (Cmd_Argc() != 2) {
		console::writeln(cl, "amweapondisable <number> | saberonly | sabermeleeonly");
		return;
	}
	int disable;
	if (!strcmp(Cmd_Argv(1), "saberonly")) {
		disable = 131063;
	} else if (!strcmp(Cmd_Argv(1), "sabermeleeonly")) {
		disable = 131059;
	} else {
		disable = atoi(Cmd_Argv(1));
	}
	if (disable < 0) {
		disable = 0;
	}
	Cvar_Set("g_weaponDisable", va("%d", disable));
}

struct Command {
	const char *name;
	std::function<void(client_t*)> func;
	const char *desc;
};

static void info(client_t*);
static void players(client_t *cl);

static Command cmds[] = 
	{ {"info", info, "show this"}
	, {"players", players, "show a list of players"}
	, {"login", login, "login to admin"}
	, {"where", where, "display origin"}
	};

static Command admin_cmds[] = 
	{ {"amban", amban, "ban a player"}
	, {"amduelfraglimit", amduelfraglimit, "change duel fraglimit"}
	, {"amduelweapondisable", amduelweapondisable, "disable weapons (in duel gametype)"}
	, {"amforcepowerdisable", amforcepowerdisable, "disable force"}
	, {"amfraglimit", amfraglimit, "change fraglimit"}
	, {"amkick", amkick, "kick a player"}
	, {"ammap", ammap, "change map, <mapname> <optional gametype> ex. ammap mp/duel1 duel"}
	, {"ammaprestart", ammaprestart, "restart map"}
	, {"amtele", amtele, "teleport"}
	, {"amtelemark", amtelemark, "teleport to telemark"}
	, {"amtimelimit", amtimelimit, "change timelimit"}
	, {"amweapondisable", amweapondisable, "disable weapons"}
	};

static struct {
	const char *alias;
	const char *orig;
} aliases[] = 
	{ {"amlogin", "login"}
	, {"showplayerid", "players"}
	, {"aminfo", "info"}
	, {"amrestart", "ammaprestart"}
	, {"ammap_restart", "ammaprestart"}
	, {"amforcedisable", "amforcepowerdisable"}
	, {"origin", "where"}
	, {"amorigin", "where"}
	//, {"noclip", "amnoclip"}
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
		buffer[0] = 0;
		Com_Printf("pad(): len is larger than buffer\n");
		return buffer;
	}
	const size_t p = PAD - len;
	if (p < 0) {
		buffer[0] = 0;
		Com_Printf("pad(): negative pad\n");
		return buffer;
	}
	size_t i = 0;
	for (; i < p && i < sizeof(buffer); i++) {
		buffer[i] = ' ';
	}
	buffer[i] = 0;
	return buffer;
}

constexpr auto INFO = R"INFO(^5jampog:^7 An engine that runtime patches basejka.
sv_pure:    ^3%-8d^7
sv_fps:     ^3%-8d^7 (your snaps: ^3%d^7) (your fps: ^3%d^7)
sv_maxRate: ^3%-8d^7 (your rate:  ^3%d^7)

commands:)INFO";

// following rate adjustments per SV_RateMsec
static int adjust_rate(int rate) {
	if (sv_maxRate->integer ) {
		if (sv_maxRate->integer < rate) {
			rate = sv_maxRate->integer;
		}
	}
	if (sv_minRate->integer) {
		if (sv_minRate->integer > rate) {
			rate = sv_minRate->integer;
		}
	}
	return rate;
}

static void info(client_t *cl) {
	console::writeln
		( cl, INFO
		, sv_pure->integer
		, sv_fps->integer, 1000 / cl->snapshotMsec, cl->clientFPS.fps()
		, sv_maxRate->integer, adjust_rate(cl->rate)
		);
	if (cl->admin.logged_in) {
		console::writeln(cl, "^5Cheat commands are enabled.^7");
	}
	for (auto &cmd: cmds) {
		console::writeln(cl, "^5%-24s^7%s", cmd.name, cmd.desc);
	}
	if (cl->admin.logged_in) {
		// console::writeln(cl, "^5%-24s^7%s", "amnoclip", "toggle noclip");
		for (auto &cmd: admin_cmds) {
			console::writeln(cl, "^5%-24s^7%s", cmd.name, cmd.desc);
		}
	}
}

static void players(client_t *cl) {
	char number[4];
	char rate[8];
	char snaps[5];
	char fps[8];
	char pad1[64];
	char pad2[64];
	char pad3[64];
	char pad4[64];
	constexpr auto NUMBER_PAD = sizeof(number);
	constexpr auto NAME_PAD = sizeof(cl->name);
	constexpr auto RATE_PAD = sizeof(rate) + 1;
	constexpr auto SNAPS_PAD = sizeof(snaps) + 3;
	str::cpy(pad1, pad("#", NUMBER_PAD));
	str::cpy(pad2, pad("name", NAME_PAD));
	str::cpy(pad3, pad("rate", RATE_PAD));
	str::cpy(pad4, pad("snaps", SNAPS_PAD));
	console::writeln(cl, "#%sname%srate%ssnaps%sfps", pad1, pad2, pad3, pad4);
	for (int i = 0; i < sv_maxclients->integer; i++) {
		client_t *it = &svs.clients[i];
		if (it->state == CS_ACTIVE) {
			Com_sprintf(number, sizeof(number), "%d", it->gentity->s.number);
			Com_sprintf(rate, sizeof(rate), "%d", adjust_rate(it->rate));
			Com_sprintf(snaps, sizeof(snaps), "%d", 1000 / it->snapshotMsec);
			Com_sprintf(fps, sizeof(fps), "%d", it->clientFPS.fps());
			str::cpy(pad1, pad(number, NUMBER_PAD));
			str::cpy(pad2, pad(it->name, NAME_PAD));
			str::cpy(pad3, pad(rate, RATE_PAD));
			str::cpy(pad4, pad(snaps, SNAPS_PAD));
			console::writeln(cl, "%s%s%s%s%s%s%s%s%s",
				number, pad1, it->name, pad2, rate, pad3, snaps, pad4, fps);
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
	#if 0
	if (!strcmp(arg0, "amnoclip")) {
		amnoclip(cl);
		return true;
	}
	#endif
	return false;
}

void jampog::command_init() {
	admin_password = Cvar_Get("admin_password", "", 0);
}
