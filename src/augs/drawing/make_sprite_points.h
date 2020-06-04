#pragma once
#include <array>
#include "augs/math/vec2.h"
#include "augs/math/transform.h"
#include "augs/math/make_rect_points.h"

namespace augs {
	using sprite_points = std::array<vec2, 4>;

	template <bool reproducible = false>
	FORCE_INLINE sprite_points make_sprite_points(
		const vec2 pos,
		const vec2i size, 
		const std::conditional_t<reproducible, real32, float> degrees
	) {
		return make_rect_points<reproducible>(pos, size, degrees);
	}

	FORCE_INLINE ltrb calc_sprite_aabb(
		const transformr where,
		const vec2i size
	) {
		return calc_vertices_aabb(
			make_sprite_points<true>(where.pos, size, where.rotation)
		);
	}
}
