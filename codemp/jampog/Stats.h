#pragma once

// fwd declare
typedef struct client_s client_t;

namespace jampog {

class Stats {
private:
	client_t *m_cl;
	int m_hits;
public:
	constexpr Stats(client_t *cl = nullptr) noexcept
	: m_cl(cl)
	, m_hits(0)
	{}
	void start_hits() noexcept;
	int hits() noexcept;
};

}