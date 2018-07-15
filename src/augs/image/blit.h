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

	template <class A, class B>
	void blit_border(
		A& into,
		const B& source_image, 
		const vec2u dst,
		const bool flip_source = false
	) {
		const auto source_size = source_image.get_size();

		if (flip_source) {
			into.pixel(dst + vec2i(-1, -1)) = source_image.pixel(vec2u(0, 0));
			into.pixel(dst + vec2i(-1, source_size.x)) = source_image.pixel(vec2u(source_size.x - 1, 0));
			into.pixel(dst + vec2i(source_size.y, source_size.x)) = source_image.pixel(source_size - vec2u(1, 1));
			into.pixel(dst + vec2i(source_size.y, -1)) = source_image.pixel(vec2u(0, source_size.y - 1));

			for (auto x = 0u; x < source_size.x; ++x) {
				into.pixel(dst + vec2u{ 0, x } - vec2u(1, 0)) = source_image.pixel(vec2u{ x, 0 });
				into.pixel(dst + vec2u{ source_size.y, x }) = source_image.pixel(vec2u{ x, source_size.y - 1 });
			}

			for (auto y = 0u; y < source_size.y; ++y) {
				into.pixel(dst + vec2u{ y, 0 } - vec2u(0, 1)) = source_image.pixel(vec2u{ 0, y });
				into.pixel(dst + vec2u{ y, source_size.x }) = source_image.pixel(vec2u{ source_size.x - 1, y });
			}
		}
		else {
			into.pixel(dst + vec2i(-1, -1)) = source_image.pixel(vec2u(0, 0));
			into.pixel(dst + vec2i(source_size.x, -1)) = source_image.pixel(vec2u(source_size.x - 1, 0));
			into.pixel(dst + vec2i(source_size.x, source_size.y)) = source_image.pixel(source_size - vec2u(1, 1));
			into.pixel(dst + vec2i(-1, source_size.y)) = source_image.pixel(vec2u(0, source_size.y - 1));

			for (auto x = 0u; x < source_size.x; ++x) {
				into.pixel(dst + vec2u{ x, 0 } - vec2u(0, 1)) = source_image.pixel(vec2u{ x, 0 });
				into.pixel(dst + vec2u{ x, source_size.y }) = source_image.pixel(vec2u{ x, source_size.y - 1 });
			}

			for (auto y = 0u; y < source_size.y; ++y) {
				into.pixel(dst + vec2u{ 0, y } - vec2u(1, 0)) = source_image.pixel(vec2u{ 0, y });
				into.pixel(dst + vec2u{ source_size.x, y }) = source_image.pixel(vec2u{ source_size.x - 1, y });
			}
		}
	}
}
