#pragma once 

#include "math/vec2.h"
#include <vector>
#include <cmath>

class melee_animation {
public:
	melee_animation(std::vector<vec2>);
	vec2 update(double factor);
	std::vector<vec2> offset_pattern;
};