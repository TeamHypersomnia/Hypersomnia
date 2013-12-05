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
			int edge_index;
			bool is_boundary;
			augmentations::vec2<> normal;

			edge points;
			augmentations::vec2<> last_undiscovered_wall;

			enum {
				RIGHT,
				LEFT
			} winding;

			discontinuity(const edge& points = edge(),
				augmentations::vec2<> last_undiscovered_wall = augmentations::vec2<>()) :
				points(points), winding(RIGHT),
				last_undiscovered_wall(last_undiscovered_wall), edge_index(0), is_boundary(false) {}
		};

		struct layer {
			/* input */
			bool postprocessing_subject;
			augmentations::graphics::pixel_32 color;
			
			b2Filter filter;
			float square_side;
			float ignore_discontinuities_shorter_than;

			/* output */
			std::vector<edge> edges;
			std::vector<augmentations::vec2<>> vertex_hits;
			std::vector<discontinuity> discontinuities;

			discontinuity* get_discontinuity(int edge_num);

			int get_num_triangles();
			triangle get_triangle(int index, augmentations::vec2<> origin);

			layer() : square_side(0.f), postprocessing_subject(false), ignore_discontinuities_shorter_than(-1.f) {}
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