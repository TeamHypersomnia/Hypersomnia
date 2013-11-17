#pragma once
#include <vector>
#include "math/vec2d.h"
#include "entity_system/component.h"
#include "graphics/pixel.h"
#include "utility/sorted_vector.h"

namespace components {
	struct visibility : public augmentations::entity_system::component {
		typedef std::pair<augmentations::vec2<>, augmentations::vec2<>> edge;

		struct triangle {
			augmentations::vec2<> points[3];
		};

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

		struct layer {
			/* input */
			bool postprocessing_subject;
			augmentations::graphics::pixel_32 color;
			
			b2Filter filter;
			float square_side;

			/* output */
			std::vector<edge> edges;
			std::vector<edge> visible_walls;
			std::vector<augmentations::vec2<>> vertex_hits;
			std::vector<discontinuity> discontinuities;

			int get_num_triangles();
			triangle get_triangle(int index, augmentations::vec2<> origin);

			layer() : square_side(0.f), postprocessing_subject(false) {}
		};

		enum layer_type {
			OBSTACLE_AVOIDANCE,
			DYNAMIC_PATHFINDING,
			CONTAINMENT
		};

		augmentations::util::sorted_vector_map<int, layer> visibility_layers;

		void add_layer(int key, const layer& val) {
			visibility_layers.add(key, val);
		}

		layer& get_layer(int key) {
			return *visibility_layers.get(key);
		}
	};
}