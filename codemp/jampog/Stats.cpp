#include "server/server.h"
#include "Entity.h"

void jampog::Stats::start() noexcept {
	m_hits = 0;
	m_shots = 0;
	m_pers_hits = m_cl->gentity->playerState->persistant[PERS_HITS];
	m_prev_shot = 0;
	m_damage = 0;
	m_kills = m_cl->gentity->playerState->persistant[PERS_SCORE];
	m_deaths = m_cl->gentity->playerState->persistant[PERS_KILLED];
}

int jampog::Stats::hits() const noexcept {
	return m_hits;
}

int jampog::Stats::shots() const noexcept {
	return m_shots;
}

int jampog::Stats::accuracy() const noexcept {
	auto h = hits();
	auto s = shots();
	if (s == 0) return 0;
	return minimum((h / float(s)) * 100.0f, 100.0f);
}

int jampog::Stats::damage() const noexcept {
	return m_damage;
}

int jampog::Stats::deaths() const noexcept {
	return m_cl->gentity->playerState->persistant[PERS_KILLED] - m_deaths;
}

int jampog::Stats::kills() const noexcept {
	return m_cl->gentity->playerState->persistant[PERS_SCORE] - m_kills;
}

void jampog::Stats::add_hit() noexcept {
	// Com_Printf("ent: %d, add_hit ", m_cl - svs.clients);
	if (m_prev_shot == m_shots) {
		// no changes in number of swings
		// duplicate hit or poke
		// Com_Printf("no change\n");
	} else {
		m_prev_shot = m_shots;
		// Com_Printf("Increase\n");
		m_hits++;
	}
}

void jampog::Stats::add_shot() noexcept {
	m_shots++;
	// Com_Printf("ent: %d, add_shot\n", m_cl - svs.clients);
}

void jampog::Stats::add_damage(int amount) noexcept {
	m_damage += amount;
}

// monitor for other hits not caught by G_LogWeaponDamage
// such as shield hits
void jampog::Stats::check_hits() noexcept {
	if (m_cl->gentity->playerState->persistant[PERS_HITS] > m_pers_hits) {
		add_hit();
		m_pers_hits = m_cl->gentity->playerState->persistant[PERS_HITS];
	}
}