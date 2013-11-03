#pragma once
#include <vector>
#include "math/vec2d.h"
#include "entity_system/component.h"
#include "graphics/pixel.h"

class ai_system;
class render_system; // for debugging

namespace components {
	struct ai : public augmentations::entity_system::component {
		bool postprocessing_subject;
		augmentations::graphics::pixel_32 visibility_color;

		float visibility_square_side;
		std::vector<augmentations::vec2<>> vision_points;

		int get_num_triangles();
		
		struct triangle {
			augmentations::vec2<> points[3];
		} get_triangle(int index, augmentations::vec2<> origin);

		ai() : visibility_square_side(0.f), postprocessing_subject(false) {}

		private:
			friend class ai_system;
			friend class render_system;

			struct discontinuity {
				augmentations::vec2<> p1, p2;
				bool visited;

				enum {
					RIGHT,
					LEFT
				} winding;

				discontinuity(augmentations::vec2<> p1 = augmentations::vec2<>(), augmentations::vec2<> p2 = augmentations::vec2<>()) : p1(p1), p2(p2), winding(RIGHT) {}
			};

			discontinuity local_minimal_discontinuity;
			std::vector<discontinuity> memorised_discontinuities;
	};
}