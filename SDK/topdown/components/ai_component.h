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

		float visibility_square_side;
		std::vector<augmentations::vec2<>> vision_points;

		int get_num_triangles();
		
		struct triangle {
			augmentations::vec2<> points[3];
		} get_triangle(int index, augmentations::vec2<> origin);

		ai() : visibility_square_side(0.f) {}
	};
}