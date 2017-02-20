#pragma once
#include "visibility_settings.h"
#include "pathfinding_settings.h"
#include "si_scaling.h"

struct all_simulation_settings {
	visibility_settings visibility;
	pathfinding_settings pathfinding;
	si_scaling si;

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