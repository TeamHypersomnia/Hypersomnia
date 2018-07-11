#pragma once
#include "augs/drawing/flip.h"
#include "augs/drawing/drawing.h"
#include "augs/math/camera_cone.h"

class interpolation_system;
struct randomizing_system;
class images_in_atlas_map;

struct specific_draw_input {
	const augs::drawer drawer;
	const images_in_atlas_map& manager;
	const double global_time_seconds;
	const flip_flags flip;
	randomizing_system& randomizing;
	const camera_cone cone;

	template <class T>
	auto make_input_for() const {
		return typename T::drawing_input(drawer, cone);
	}
};

struct draw_renderable_input : specific_draw_input {
	const interpolation_system& interp;
};
