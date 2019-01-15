#pragma once
#include <array>
#include "augs/math/vec2.h"
#include "augs/build_settings/compiler_defines.h"
#include "augs/math/transform.h"

namespace augs {
	using sprite_points = std::array<vec2, 4>;

	template <bool reproducible = false>
	FORCE_INLINE auto make_sprite_points(
		const vec2 pos, 
		const vec2i size, 
		const std::conditional_t<reproducible, real32, float> degrees
	) {
		sprite_points v;

		v[0] = v[1] = v[2] = v[3] = -size / 2;

		// v[0];
		v[1].x += size.x;

		v[2].x += size.x;
		v[2].y += size.y;

		v[3].y += size.y;

		const auto radians = DEG_TO_RAD<remove_cref<decltype(degrees)>> * degrees;

		if constexpr(reproducible) {
			const auto s = static_cast<float>(repro::sin(radians));
			const auto c = static_cast<float>(repro::cos(radians));

			for (auto& vv : v) {
				const auto new_x = vv.x * c - vv.y * s;
				const auto new_y = vv.x * s + vv.y * c;

				vv.x = new_x;
				vv.y = new_y;

				vv += pos;
			}
		}
		else {
			const auto s = static_cast<float>(std::sin(radians));
			const auto c = static_cast<float>(std::cos(radians));

			for (auto& vv : v) {
				const auto new_x = vv.x * c - vv.y * s;
				const auto new_y = vv.x * s + vv.y * c;

				vv.x = new_x;
				vv.y = new_y;

				vv += pos;
			}
		}

		return v;
	}

	FORCE_INLINE ltrb sprite_aabb(
		const transformr where,
		const vec2i size
	) {
		return augs::get_aabb(
			augs::make_sprite_points<true>(where.pos, size, where.rotation)
		);
	}
}
