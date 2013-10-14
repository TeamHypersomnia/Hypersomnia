#pragma once
#include <vector>
#include "math/vec2d.h"
#include "entity_system/component.h"

namespace components {
	struct ai : public augmentations::entity_system::component {
		struct vision_triangle {
			augmentations::vec2<> points[3];
		};

		float visibility_radius;
		std::vector<vision_triangle> visible_area;
	};
}