#include "zmq.h"
#include <zmqpp/zmqpp.hpp>
#include <server/server.h>
#include <memory>
#include <string_view>
#include <array>
#include <tuple>
#include "util.h"
#include "Player.h"

namespace console = jampog::console;

static std::unique_ptr<zmqpp::context> ctx;
static std::unique_ptr<zmqpp::socket> subscriber;

static std::tuple<std::array<client_t*, MAX_CLIENTS>, int> lookup_client(netadr_t a) {
	std::array<client_t*, MAX_CLIENTS> clients;
	int matches = 0;
	for (auto i = 0; i < sv_maxclients->integer; i++) {
		auto cl = svs.clients + i;
		if (cl->state != CS_ACTIVE) continue;
		if (NET_CompareBaseAdr(cl->netchan.remoteAddress, a)) {
			clients[matches++] = cl;
		}
	}
	return {clients, matches};
}

static void authorize(std::string ip, std::string snowflake, std::string username) {
	Com_Printf("Received Authorization request: %s\n", ip.data());
	netadr_t a;
	if (!Sys_StringToAdr(ip.data(), &a)) {
		return Com_Error(ERR_FATAL, "authorize() failed to convert IP: \"%s\"", ip.data());
	}
	Com_Printf("Parsed IP: %s\n", NET_AdrToString(a));
	auto res = lookup_client(a);
	auto clients = std::get<0>(res);
	auto matches = std::get<1>(res);
	if (matches > 1) {
		for (auto i = 0; i < matches; i++) {
			auto cl = clients[i];
			console::writeln(cl, "^1Cannot authorize, multiple clients from same IP^7");
		}
	} else if (matches == 1) {
		auto cl = clients[0];
		console::writeln(cl, "^2You are now authorized: %s^7", username.c_str());
		jampog::Player::get(cl).discord.login(std::move(snowflake), std::move(username));
	} else {
		Com_Printf("NO MATCHES\n");
	}
}

void jampog::zmq::init() {
	ctx = std::make_unique<zmqpp::context>();
	subscriber = std::make_unique<zmqpp::socket>(*ctx, zmqpp::socket_type::pull);
	subscriber->connect("tcp://127.0.0.1:3030");
}

void jampog::zmq::shutdown() {
	subscriber.release();
	ctx.release();
}

void jampog::zmq::check_events() {
	zmqpp::message msg;
	constexpr auto dont_block = true;
	if (subscriber->receive(msg, dont_block)) {
		auto cmd = msg.get(0);
		if (cmd == "auth") {
			authorize(msg.get(1), msg.get(2), msg.get(3));
		}
	}
}