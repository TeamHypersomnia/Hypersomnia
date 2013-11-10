#pragma once
#include <vector>
#include "math/vec2d.h"
#include "entity_system/component.h"
#include "graphics/pixel.h"
#include "utility/sorted_vector.h"

class ai_system;
class render_system; // for debugging

namespace components {
	struct ai : public augmentations::entity_system::component {
		typedef std::pair<augmentations::vec2<>, augmentations::vec2<>> edge;

		struct visibility {
			enum type {
				OBSTACLE_AVOIDANCE,
				DYNAMIC_PATHFINDING
			};

			bool postprocessing_subject;
			augmentations::graphics::pixel_32 color;
			
			b2Filter filter;

			float square_side;
			std::vector<edge> edges;

			int get_num_triangles();

			struct triangle {
				augmentations::vec2<> points[3];
			} get_triangle(int index, augmentations::vec2<> origin);

			struct discontinuity {
				edge points;
				bool visited;

				enum {
					RIGHT,
					LEFT
				} winding;

				discontinuity(edge points = edge()) : points(points), winding(RIGHT) {}
			};

			discontinuity local_minimal_discontinuity;
			std::vector<discontinuity> memorised_discontinuities;
			
			std::vector<edge> memorised_walls;
			std::vector<edge> memorised_undiscovered_walls;

			visibility() : square_side(0.f), postprocessing_subject(false) {}
		};

		augmentations::util::sorted_vector_map<int, visibility> visibility_requests;
		
		void add_request(int key, const visibility& val) {
			visibility_requests.add(key, val);
		}

		visibility& get_visibility(int key) {
			return *visibility_requests.get(key);
		}
	};
}