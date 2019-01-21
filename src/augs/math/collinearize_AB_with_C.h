#pragma once
#include "augs/math/math.h"

template <class F>
real32 collinearize_AB_with_C(
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

	const auto oc_radius = crosshair_vector.length();

	const auto intersection = circle_ray_intersection(
		A_barrel_center, 
		B_muzzle + (B_muzzle - A_barrel_center).set_length(5000.f), 
		O_center_of_rotation, 
		oc_radius
	);
	
	if (!intersection.hit) {
		return 0.f;
	}

	const auto G = intersection.intersection;
	const auto CG = C_crosshair - G;
	const auto CG_degress = CG.is_zero() ? real32(90.f) : CG.degrees();
	const auto AG = A_barrel_center - G;

	const auto final_angle = 2 * (CG_degress - AG.degrees());

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