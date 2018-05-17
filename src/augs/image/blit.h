#pragma once
#include "augs/math/vec2.h"

namespace augs {
	template <class A, class B>
	void blit(
		A& into,
		const B& source_image, 
		const vec2u dst,
		const bool flip_source = false,
		const bool additive = false
	) {
		const auto source_size = source_image.get_size();

		if (!additive) {
			if (flip_source) {
				for (auto y = 0u; y < source_size.y; ++y) {
					for (auto x = 0u; x < source_size.x; ++x) {
						into.pixel(dst + vec2u{ y, x }) = source_image.pixel(vec2u{ x, y });
					}
				}
			}
			else {
				for (auto y = 0u; y < source_size.y; ++y) {
					for (auto x = 0u; x < source_size.x; ++x) {
						into.pixel(dst + vec2u{ x, y }) = source_image.pixel(vec2u{ x, y });
					}
				}
			}
		}
		else {
			if (flip_source) {
				for (auto y = 0u; y < source_size.y; ++y) {
					for (auto x = 0u; x < source_size.x; ++x) {
						into.pixel(dst + vec2u{ y, x }) += source_image.pixel(vec2u{ x, y });
					}
				}
			}
			else {
				for (auto y = 0u; y < source_size.y; ++y) {
					for (auto x = 0u; x < source_size.x; ++x) {
						into.pixel(dst + vec2u{ x, y }) += source_image.pixel(vec2u{ x, y });
					}
				}
			}
		}
	}

}
