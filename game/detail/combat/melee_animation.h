#pragma once 

#include "math/vec2.h"
#include "game/components/transform_component.h"

#include <vector>
#include <cmath>


class melee_animation {
public:
	components::transform calculate_intermediate_transform(double factor) const;
	std::vector<components::transform> offset_pattern;
};