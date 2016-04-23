#pragma once 

#include "math/vec2.h"
#include "game/components/transform_component.h"

#include <vector>
#include <cmath>


class melee_animation {
public:
	melee_animation(std::vector<components::transform>);
	components::transform update(double factor);
	std::vector<components::transform> offset_pattern;
};