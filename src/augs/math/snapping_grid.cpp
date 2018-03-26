#include "augs/math/snapping_grid.h"

vec2 snapping_grid::snap(const vec2 position) const {
	auto rounded_position = vec2i(position);

	rounded_position /= unit_pixels;
	rounded_position *= unit_pixels;

	return vec2(rounded_position);
}

float snapping_grid::snap(const float rotation) const {
	auto rounded_rot = static_cast<int>(rotation);

	rounded_rot /= unit_degrees;

	return static_cast<float>(rounded_rot);
}

transform snapping_grid::snap(transform t) const {
	t.pos = snap(t.pos);
	t.rotation = snap(t.rotation);

	return t;
}
