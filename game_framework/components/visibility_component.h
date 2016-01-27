#pragma once
#include <vector>
#include "math/vec2.h"
#include <Box2D\Dynamics\b2Fixture.h>

#include "graphics/pixel.h"
#include "misc/sorted_vector.h"
#include "misc/timer.h"

namespace components {
	struct visibility  {
		typedef std::pair<vec2, vec2> edge;

		struct triangle {
			vec2 points[3];
		};

		struct discontinuity {
			int edge_index;
			bool is_boundary;
			vec2 normal;

			edge points;
			vec2 last_undiscovered_wall;

			enum {
				RIGHT,
				LEFT
			} winding;

			discontinuity(const edge& points = edge(),
				vec2 last_undiscovered_wall = vec2()) :
				points(points), winding(RIGHT),
				last_undiscovered_wall(last_undiscovered_wall), edge_index(0), is_boundary(false) {}
		};

		struct layer {
			/* input */
			bool postprocessing_subject;
			augs::rgba color;
			
			b2Filter filter;
			float square_side;
			float ignore_discontinuities_shorter_than;

			vec2 offset;

			/* output */
			std::vector<edge> edges;

			/* first: edge index, second: location */
			std::vector<std::pair<int, vec2>> vertex_hits;
			std::vector<discontinuity> discontinuities;

			/* segments that denote narrow areas */
			std::vector<edge> marked_holes;

			discontinuity* get_discontinuity_for_edge(int edge_num);
			discontinuity* get_discontinuity(int disc_num);
			
			int get_num_discontinuities() {
				return discontinuities.size();
			}

			int get_num_triangles();
			triangle get_triangle(int index, vec2 origin);
			std::vector<vec2> get_polygon(float distance_epsilon, vec2 expand_origin, float expand_mult);

			layer() : square_side(0.f), postprocessing_subject(false), ignore_discontinuities_shorter_than(-1.f) {}
		};

		enum layer_type {
			OBSTACLE_AVOIDANCE,
			DYNAMIC_PATHFINDING,
			CONTAINMENT
		};

		augs::sorted_associative_vector<int, layer> visibility_layers;

		void add_layer(int key, const layer& val) {
			visibility_layers.add(key, val);
		}

		layer& get_layer(int key) {
			return *visibility_layers.get(key);
		}

		augs::timer interval_timer;
		float interval_ms;
		visibility() : interval_ms(-1.f) {}
	};
}