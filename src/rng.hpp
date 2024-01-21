#pragma once

#include <random>

namespace rng {
	static inline std::random_device s_Device;
	static inline std::mt19937 s_Generator(s_Device());
}// namespace rng
