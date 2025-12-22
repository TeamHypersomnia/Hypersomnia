#pragma once
#include "augs/ensure.h"
#include "augs/math/vec2.h"
#include "augs/math/rects.h"
#include "augs/graphics/rgba.h"

struct intersection_output {
	bool hit = false;
	vec2 intersection;
};

bool point_in_rect(
	vec2 center,
	real32 rotation,
	vec2 size,
	vec2 point
);

intersection_output rectangle_ray_intersection(
	const vec2 a,
	const vec2 b,
	const ltrb rectangle
);

intersection_output circle_ray_intersection(
	const vec2 a,
	const vec2 b,
	const vec2 circle_center,
	const real32 circle_radius
);

intersection_output segment_segment_intersection(
	const vec2 a1,
	const vec2 a2,
	const vec2 b1,
	const vec2 b2
);

std::vector<vec2> generate_circle_points(
	const real32 radius,
	const real32 last_angle_in_degrees,
	const real32 starting_angle_in_degrees,
	const unsigned int number_of_points
);

vec2 position_rectangle_around_a_circle(
	const real32 circle_radius,
	const vec2 rectangle_size,
	const real32 position_at_degrees
);

vec2 clamp_with_raycast(
	const vec2 point,
	const vec2 rectangle_size
);