#include "augs/templates/algorithm_templates.h"
#include "augs/math/snapping_grid.h"

vec2 snapping_grid::get_snapping_delta(const ltrb aabb) const {
	std::array<vec2, 4> candidate_corners;

	{
		const auto current = aabb.left_top();
		const auto snapped = get_snapped(current);

		candidate_corners[0] = snapped - current;
	}

	{
		const auto current = aabb.right_top();
		const auto snapped = get_snapped(current);

		candidate_corners[1] = snapped - current;
	}

	{
		const auto current = aabb.right_bottom();
		const auto snapped = get_snapped(current);

		candidate_corners[2] = snapped - current;
	}

	{
		const auto current = aabb.left_bottom();
		const auto snapped = get_snapped(current);

		candidate_corners[3] = snapped - current;
	}

	return minimum_of(candidate_corners);
}

vec2i snapping_grid::get_snapped(const vec2 position) const {
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

	const auto predicate = [position](
		const vec2i a,
	   	const vec2i b
	) {
		return (vec2(a) - position) < (vec2(b) - position);
	};

	return minimum_of(corners, predicate);
}

int snapping_grid::get_snapped(const float degrees) const {
	auto rounded_degrees = static_cast<int>(std::round(degrees)) + 180;
	const auto unit = static_cast<int>(unit_degrees);

	rounded_degrees /= unit;
	rounded_degrees *= unit;

	return rounded_degrees - 180;
}
