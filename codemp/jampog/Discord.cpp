#include "Discord.h"
#include <memory>

jampog::Discord::Discord() noexcept
	: m_snowflake{}
	, m_username{}
	, m_authorized{false}
{}

void jampog::Discord::login(std::string snowflake, std::string username) noexcept {
	m_snowflake = std::move(snowflake);
	m_username = std::move(username);
	m_authorized = true;
}

bool jampog::Discord::authorized() const noexcept {
	return m_authorized;
}

const char *jampog::Discord::username() const noexcept {
	return m_username.c_str();
}

const char *jampog::Discord::snowflake() const noexcept {
	return m_snowflake.c_str();
}