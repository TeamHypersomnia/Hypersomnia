#pragma once
#include <array>

#include "augs/math/rects.h"
#include "augs/drawing//drawing.h"
#include "augs/misc/constant_size_vector.h"
#include "augs/gui/gui_traversal_structs.h"
#include "augs/texture_atlas/atlas_entry.h"

augs::constant_size_vector<std::array<vec2, 2>, 2> get_connecting_pixel_lines(
	const ltrb& origin, 
	const ltrb& target
);

inline void draw_pixel_line_connector(
	const augs::drawer_with_default drawer,
	const ltrb& origin,
	const ltrb& target,
	const rgba col
) {
	for (const auto& l : get_connecting_pixel_lines(origin, target)) {
		drawer.line(l[0], l[1], 1.f, col);
	}
}
