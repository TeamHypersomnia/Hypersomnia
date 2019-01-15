#include "augs/templates/algorithm_templates.h"
#include "augs/math/snapping_grid.h"

vec2 snapping_grid::get_snapping_delta(const ltrb aabb) const {
	std::array<vec2, 4> corner_deltas;

	{
		const auto current = vec2i(aabb.left_top().round_fract());
		const auto corner = get_snapping_corner(current);

		corner_deltas[0] = corner - current;
	}

	{
		const auto current = vec2i(aabb.right_top().round_fract());
		const auto corner = get_snapping_corner(current);

		corner_deltas[1] = corner - current;
	}

	{
		const auto current = vec2i(aabb.right_bottom().round_fract());
		const auto corner = get_snapping_corner(current);

		corner_deltas[2] = corner - current;
	}

	{
		const auto current = vec2i(aabb.left_bottom().round_fract());
		const auto corner = get_snapping_corner(current);

		corner_deltas[3] = corner - current;
	}

	return minimum_of(corner_deltas);
}

vec2i snapping_grid::get_snapping_corner(const vec2 position) const {
	std::array<vec2i, 4> corners;

	corners[0] = [this, position](){
		auto v = vec2i(vec2(position).round_fract());

		v /= unit_pixels;
		v *= unit_pixels;

		return v;
	}();

	corners[1] = corners[0] + vec2i(unit_pixels, 0);
	corners[2] = corners[0] + vec2i(unit_pixels, unit_pixels);
	corners[3] = corners[0] + vec2i(0, unit_pixels);

	const auto rounded_position = vec2i(vec2(position).round_fract());

	const auto predicate = [rounded_position](
		const vec2i a,
	   	const vec2i b
	) {
		return (a - rounded_position) < (b - rounded_position);
	};

	const auto closest_corner = minimum_of(corners, predicate);
	return closest_corner;
}

int snapping_grid::get_snapped(const float degrees) const {
	auto rounded_degrees = static_cast<int>(repro::round(degrees)) + 180;
	const auto unit = static_cast<int>(unit_degrees);

	rounded_degrees /= unit;
	rounded_degrees *= unit;

	return rounded_degrees - 180;
}
