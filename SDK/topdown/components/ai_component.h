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


			bool generate_navigation_info;

			std::vector<edge> visible_walls, undiscovered_walls;

			struct discontinuity {
				edge points;
				augmentations::vec2<> last_undiscovered_wall;

				enum {
					RIGHT,
					LEFT
				} winding;
				
				discontinuity(const edge& points = edge(),
					augmentations::vec2<> last_undiscovered_wall = augmentations::vec2<>()) :
					points(points), winding(RIGHT),
					last_undiscovered_wall(last_undiscovered_wall) {}
			};

			std::vector<discontinuity> undiscovered_discontinuities;

			visibility() : square_side(0.f), postprocessing_subject(false), generate_navigation_info(false) {}
		};

		ai() : is_finding_a_path(false), avoidance_width(0.f) {}

		bool is_finding_a_path;
		augmentations::vec2<> target, navigate_to;
		float avoidance_width;

		augmentations::util::sorted_vector_map<int, visibility> visibility_requests;
		
		void add_request(int key, const visibility& val) {
			visibility_requests.add(key, val);
		}

		visibility& get_visibility(int key) {
			return *visibility_requests.get(key);
		} 

		void clear_pathfinding_info() {
			auto& info = *visibility_requests.get(visibility::DYNAMIC_PATHFINDING);
			info.undiscovered_discontinuities.clear();
			info.visible_walls.clear();
			info.undiscovered_walls.clear();
		}
	};
}