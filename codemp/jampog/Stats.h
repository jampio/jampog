#pragma once

// fwd declare
typedef struct client_s client_t;

namespace jampog {

class Stats {
private:
	client_t *m_cl;
	int m_hits;
	int m_shots;
	int m_pers_hits;
	int m_prev_shot;
public:
	constexpr Stats(client_t *cl = nullptr) noexcept
	: m_cl(cl)
	, m_hits(0)
	, m_shots(0)
	, m_pers_hits(0)
	, m_prev_shot(0)
	{}
	void start() noexcept;
	int hits() const noexcept;
	int shots() const noexcept;
	int accuracy() const noexcept;
	void add_hit() noexcept;
	void add_shot() noexcept;
	void check_hits() noexcept;
};

}