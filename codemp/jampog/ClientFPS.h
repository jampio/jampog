#pragma once

namespace jampog {

class ClientFPS {
private:
	int m_count;
	int m_time_start_ms;
	int m_fps;
public:
	constexpr ClientFPS() 
		: m_count(0)
		, m_time_start_ms(0)
		, m_fps(0)
	{}
	constexpr int fps() const {
		return m_fps;
	}
	constexpr void update(int time_ms) {
		if (time_ms - m_time_start_ms >= 1000) {
			m_fps = m_count;
			m_count = 0;
			m_time_start_ms = time_ms;
		} else {
			m_count++;
		}
	}
};

}