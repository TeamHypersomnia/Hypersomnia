#pragma once
#include <array>
#include "augs/math/vec2.h"
#include "augs/build_settings/platform_defines.h"

namespace augs {
	FORCE_INLINE auto make_sprite_points(
		const vec2 pos, 
		const vec2i size, 
		const float rotation_degrees
	) {
		std::array<vec2, 4> v;

		v[0] = v[1] = v[2] = v[3] = pos - (size / 2);

		// v[0];
		v[1].x += size.x;

		v[2].x += size.x;
		v[2].y += size.y;

		v[3].y += size.y;

		v[0].rotate(rotation_degrees, pos);
		v[1].rotate(rotation_degrees, pos);
		v[2].rotate(rotation_degrees, pos);
		v[3].rotate(rotation_degrees, pos);

		return v;
	}
}
