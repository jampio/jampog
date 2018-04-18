#include "server/server.h"

void jampog::Stats::start_hits() noexcept {
	m_hits = m_cl->gentity->playerState->persistant[PERS_HITS];
}

int jampog::Stats::hits() noexcept {
	return m_cl->gentity->playerState->persistant[PERS_HITS] - m_hits;
}