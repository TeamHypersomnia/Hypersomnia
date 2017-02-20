#pragma once
#include "augs/misc/input_context.h"
#include "visibility_settings.h"
#include "pathfinding_settings.h"

struct all_simulation_settings {
	visibility_settings visibility;
	pathfinding_settings pathfinding;

	template <class Archive>
	void serialize(Archive& ar) {
		ar(
			CEREAL_NVP(screen_size),
			CEREAL_NVP(input),
			CEREAL_NVP(visibility),
			CEREAL_NVP(pathfinding)
		);
	}
};