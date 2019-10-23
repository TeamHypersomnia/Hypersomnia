#pragma once
#include "augs/build_settings/compiler_defines.h"
#include "augs/templates/remove_cref.h"
#include "augs/math/vec2.h"

namespace augs {
	template <bool reproducible = true, class P, class S>
	FORCE_INLINE auto make_rect_points(
		const P& pos, 
		const basic_vec2<S> size, 
		const std::conditional_t<reproducible, real32, float> degrees
	) {
		std::array<vec2, 4> v;

		{
			const auto h_size = -size / 2;

			v[0] = h_size;
			v[1] = h_size;
			v[2] = h_size;
			v[3] = h_size;
		}
	
		// v[0];
		v[1].x += size.x;

		v[2].x += size.x;
		v[2].y += size.y;

		v[3].y += size.y;

		const auto radians = DEG_TO_RAD<remove_cref<decltype(degrees)>> * degrees;

		real32 s = 0.f;
		real32 c = 0.f;

		if constexpr(reproducible) {
			repro::sincosf(radians, s, c);
		}
		else {
			s = static_cast<real32>(std::sin(radians));
			c = static_cast<real32>(std::cos(radians));
		}

		for (auto& vv : v) {
			const auto new_x = vv.x * c - vv.y * s;
			const auto new_y = vv.x * s + vv.y * c;

			vv.x = new_x;
			vv.y = new_y;

			vv += pos;
		}

		return v;
	}
}
