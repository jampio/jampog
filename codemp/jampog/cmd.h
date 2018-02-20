#pragma once

typedef struct client_s client_t;

namespace jampog {

	struct Admin {
		bool logged_in;
	};

	bool command(client_t *cl);
	void command_init();
}
