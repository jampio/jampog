#pragma once

#include <optional>
#include <array>
#include <utility>

namespace jampog {

class Telemark {
private:
	std::optional<std::array<float, 3>> m_mark;
public:
	constexpr Telemark() noexcept
		: m_mark()
	{}
	constexpr bool is_marked() const noexcept {
		return m_mark.has_value();
	}
	void set(float x, float y, float z) noexcept {
		std::array<float, 3> arr{x, y, z};
		m_mark.emplace(std::move(arr));
	}
	constexpr const float *get() const {
		return m_mark.value().data();
	}
};

}