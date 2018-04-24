#pragma once

#include <string>

namespace jampog {

class Discord {
public:
	Discord() noexcept;
	void login(std::string snowflake, std::string username) noexcept;
	bool authorized() const noexcept;
	const char *username() const noexcept;
	const char *snowflake() const noexcept;
private:
	std::string m_snowflake; // may change to uint64_t later
	std::string m_username;
	bool m_authorized;
};

}