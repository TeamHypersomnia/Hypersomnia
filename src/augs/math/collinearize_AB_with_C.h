#pragma once
#include "augs/math/math.h"

template <class F>
real32 collinearize_AB_with_C(
	vec2 A_barrel_center, 
	vec2 B_muzzle, 
	vec2 C_crosshair,
	F debug_line_drawer
) {
	if (C_crosshair.length_sq() < B_muzzle.length_sq() + 4.f) {
		C_crosshair.set_length(B_muzzle.length() + 2);
	}

	auto ray_a = A_barrel_center - (B_muzzle - A_barrel_center).set_length(5000.f);
	auto ray_b = A_barrel_center + (B_muzzle - A_barrel_center).set_length(5000.f);

	const auto intersection = circle_ray_intersection(
		ray_b, 
		ray_a, 
		vec2(0, 0), 
		C_crosshair.length()
	);

	if (!intersection.hit) {
		return 0.f;
	}

	const auto I = intersection.intersection;
	const auto final_angle = C_crosshair.degrees() - I.degrees();

	const auto O_center_of_rotation = vec2(0, 0);

	debug_line_drawer(cyan, O_center_of_rotation, C_crosshair);
	debug_line_drawer(red, O_center_of_rotation, A_barrel_center);
	debug_line_drawer(red, O_center_of_rotation, B_muzzle);
	debug_line_drawer(yellow, O_center_of_rotation, I);

	debug_line_drawer(green, I, A_barrel_center);
	debug_line_drawer(green, I, C_crosshair);

	A_barrel_center.rotate(final_angle, O_center_of_rotation);
	B_muzzle.rotate(final_angle, O_center_of_rotation);

	debug_line_drawer(red, O_center_of_rotation, A_barrel_center);
	debug_line_drawer(red, O_center_of_rotation, B_muzzle);

	return final_angle;
}