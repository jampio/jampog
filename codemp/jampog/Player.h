#pragma once

#include <server/server.h>
#include "Admin.h"
#include "Discord.h"
#include "ClientFPS.h"
#include "Telemark.h"
#include "Stats.h"
#include "Entity.h"

namespace jampog {

class Player {
private:
	Player(const Player&) = default;
	Player& operator=(const Player&) = default;
public:
	Discord discord{};
	Admin admin{};
	ClientFPS clientFPS{};
	Telemark telemark{};
	Stats stats{};

	bool nonsolid{true};
	bool noduelInProgress{true};
	bool noduelevent{false};
	bool drawduelers{true};
	bool drawothers{false};

	void toggle_nonsolid() noexcept { nonsolid = !nonsolid; }
	void toggle_noduelInProgress() noexcept { noduelInProgress = !noduelInProgress; }
	void toggle_noduelevent() noexcept { noduelevent = !noduelevent; }
	void toggle_drawduelers() noexcept { drawduelers = !drawduelers; }
	void toggle_drawothers() noexcept { drawothers = !drawothers; }

	Player(client_t *cl = nullptr) : stats{cl} {}
	operator client_t *() const noexcept;
	void println_red(const char *str) const noexcept;
	void println_green(const char *str) const noexcept;

	static void reset(client_t *cl) noexcept;
	static Player& get(int id) noexcept;
	static Player& get(client_t *cl) noexcept;
	static Player& get(sharedEntity_t *ent) noexcept;
	static Player& get(Entity ent) noexcept;
};

}