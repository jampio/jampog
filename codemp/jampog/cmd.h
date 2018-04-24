#pragma once

typedef struct client_s client_t;

namespace jampog {
	bool command(client_t *cl);
	void command_init();
}
