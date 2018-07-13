#pragma once
#include "augs/math/vec2.h"
#include "augs/math/rects.h"
#include "augs/build_settings/platform_defines.h"

namespace augs {
	FORCE_INLINE ltrbi visible_grid_cells_detail(
		const vec2i& tile_size, 
		const vec2i& times, // grid aabb over tile size
		const ltrbi& s, // grid aabb
		const ltrbi& c // camera aabb
	) {
		/* We start from assumption that the grid is completely within the camera */
		auto v = ltrbi(vec2i(0, 0), times);

		/* But, right edge of the object might exceed right bound of camera */
		{
			const auto diff = s.r - c.r;

			if (diff > 0) {
				/* If the difference exceeds the tile size, the result of integer division becomes bigger than 0, in which case we already take 1 from times.x */
				v.r -= diff / tile_size.x;
			}
		}

		v.b -= std::max(0, s.b - c.b) / tile_size.y;

		/* But, left edge of the camera might exceed left bound of object */
		{
			const auto diff = c.l - s.l;

			if (diff > 0) {
				/* If the difference exceeds the tile size, the result of integer division becomes bigger than 0, in which case we already put 1 as the beginning point. */
				v.l = diff / tile_size.x;
			}
		}

		v.t = std::max(0, c.t - s.t) / tile_size.y;

		return v;
	}

	FORCE_INLINE ltrbi visible_grid_cells(
		const vec2i& tile_size, 
		const ltrbi& grid_aabb,
		const ltrbi& cam_aabb
	) {
		return visible_grid_cells_detail(
			tile_size, 
			grid_aabb.get_size() / tile_size, 
			grid_aabb, 
			cam_aabb
		);
	}
}
