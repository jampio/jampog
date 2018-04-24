#include "Player.h"
#include <memory>

static jampog::Player players[MAX_CLIENTS];

jampog::Player::operator client_t *() const noexcept {
	return svs.clients + (this - players);
}

void jampog::Player::reset(client_t *cl) noexcept {
	players[cl - svs.clients] = Player{cl};
}

jampog::Player& jampog::Player::get(int id) noexcept {
	return players[id];
}

jampog::Player& jampog::Player::get(client_t *cl) noexcept {
	return get(cl - svs.clients);
}

jampog::Player& jampog::Player::get(sharedEntity_t *ent) noexcept {
	return get(SV_NumForGentity(ent));
}

jampog::Player& jampog::Player::get(jampog::Entity ent) noexcept {
	return get(ent.number());
}

void jampog::Player::println_red(const char *str) const noexcept {
	SV_SendServerCommand(this->operator client_t*(), "print \"^1%s^7\n\"", str);
}

void jampog::Player::println_green(const char *str) const noexcept {
	SV_SendServerCommand(this->operator client_t*(), "print \"^2%s^7\n\"", str);
}