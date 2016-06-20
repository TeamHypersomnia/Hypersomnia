#pragma once
#include <vector>
#include <unordered_map>
#include "math/vec2.h"
#include <Box2D\Dynamics\b2Fixture.h>

#include "graphics/pixel.h"

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

		struct full_visibility_info {
			/* input */
			augs::rgba color;
			
			b2Filter filter;
			float square_side = 0.f;
			float ignore_discontinuities_shorter_than = -1.f;

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
		};

		struct line_of_sight_info {
			augs::rgba color;

			b2Filter obstruction_filter;
			b2Filter candidate_filter;
			float maximum_distance = 0.f;

			bool test_items = false;
			bool test_sentiences = false;
			bool test_attitudes = false;
			bool test_dangers = false;

			std::set<entity_id> visible_items;
			std::set<entity_id> visible_sentiences;
			std::set<entity_id> visible_attitudes;
			std::set<entity_id> visible_dangers;
			
			bool sees(entity_id) const;
		};

		enum layer_type {
			DYNAMIC_PATHFINDING,
			LINE_OF_SIGHT
		};

		std::unordered_map<layer_type, full_visibility_info> full_visibility_layers;
		std::unordered_map<layer_type, line_of_sight_info> line_of_sight_layers;
	};
}