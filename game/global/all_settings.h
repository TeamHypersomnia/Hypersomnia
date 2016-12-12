#pragma once
#include "input_context.h"
#include "visibility_settings.h"
#include "pathfinding_settings.h"

struct all_settings {
	int enable_interpolation = false;

	float listener_maximum_radius = 1920.f * 5.f;
	visibility_settings visibility;
	pathfinding_settings pathfinding;

	template <class Archive>
	void serialize(Archive& ar) {
		ar(
			CEREAL_NVP(enable_interpolation),

			CEREAL_NVP(screen_size),
			CEREAL_NVP(input),
			CEREAL_NVP(visibility),
			CEREAL_NVP(pathfinding)
		);
	}
};