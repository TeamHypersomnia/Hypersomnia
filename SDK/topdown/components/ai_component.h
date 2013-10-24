#pragma once
#include <vector>
#include "math/vec2d.h"
#include "entity_system/component.h"

namespace components {
	struct ai : public augmentations::entity_system::component {
		struct debug_line {
			debug_line(augmentations::vec2<> a, augmentations::vec2<> b) : a(a), b(b) {}

			augmentations::vec2<> a, b;
		};
		std::vector<debug_line> lines;

		float visibility_radius;
		float visibility_angle;
		ai() : visibility_radius(0.f) {}
	};
}