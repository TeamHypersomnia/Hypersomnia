#pragma once
#include <cmath>
#include <algorithm>

namespace augs {
	enum class easing_type {
		LINEAR,
		SQRT,
		EXPONENTIAL
	};

	inline float easing(
		const easing_type type,
		const float start,
		const float end,
		const float passed_ratio,
		const float halve_per_ratio = -1.0f
	) {
		const auto clamped = std::clamp(passed_ratio, 0.0f, 1.0f);

		float r;

		switch (type) {
			case easing_type::LINEAR:
				r = clamped;
				break;
			case easing_type::SQRT:
				r = std::sqrt(clamped);
				break;
			case easing_type::EXPONENTIAL:
				if (halve_per_ratio > 0.0f) {
					r = 1.0f - std::pow(0.5f, clamped / halve_per_ratio);
				}
				else {
					r = clamped;
				}
				break;
			default:
				r = clamped;
				break;
		}

		return start + (end - start) * r;
	}
}
