#pragma once
#include "augs/ensure.h"
#include "augs/math/vec2.h"
#include "augs/math/rects.h"
#include "augs/graphics/rgba.h"

struct intersection_output {
	bool hit = false;
	vec2 intersection;
};

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

template <class F>
float colinearize_AB_with_C(
	const vec2 O_center_of_rotation, 
	vec2 A_barrel_center, 
	vec2 B_muzzle, 
	vec2 C_crosshair,
	F debug_line_drawer
) {
	auto crosshair_vector = C_crosshair - O_center_of_rotation;
	const auto barrel_vector = B_muzzle - O_center_of_rotation;

	if (crosshair_vector.is_epsilon(1.f)) {
		crosshair_vector.set(1, 0);
	}

	if (crosshair_vector.length() < barrel_vector.length() + 1.f) {
		return crosshair_vector.degrees();
	}

	C_crosshair = O_center_of_rotation + crosshair_vector;

	const float oc_radius = crosshair_vector.length();

	const auto intersection = circle_ray_intersection(B_muzzle, A_barrel_center, O_center_of_rotation, oc_radius);
	const bool has_intersection = intersection.hit;

	ensure(has_intersection);

	const auto G = intersection.intersection;
	const auto CG = C_crosshair - G;
	const auto AG = A_barrel_center - G;

	const auto final_angle = 2 * (CG.degrees() - AG.degrees());

	debug_line_drawer(cyan, O_center_of_rotation, C_crosshair);
	debug_line_drawer(red, O_center_of_rotation, A_barrel_center);
	debug_line_drawer(red, O_center_of_rotation, B_muzzle);
	debug_line_drawer(yellow, O_center_of_rotation, G);

	debug_line_drawer(green, G, A_barrel_center);
	debug_line_drawer(green, G, C_crosshair);

	A_barrel_center.rotate(final_angle, O_center_of_rotation);
	B_muzzle.rotate(final_angle, O_center_of_rotation);

	debug_line_drawer(red, O_center_of_rotation, A_barrel_center);
	debug_line_drawer(red, O_center_of_rotation, B_muzzle);

	debug_line_drawer(white, A_barrel_center - (B_muzzle - A_barrel_center) * 100, B_muzzle + (B_muzzle - A_barrel_center)*100);

	return final_angle;
}