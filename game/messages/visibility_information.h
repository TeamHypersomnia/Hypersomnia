#pragma once
#include "message.h"
#include <set>
#include <vector>
#include "augs/graphics/pixel.h"
#include "3rdparty/Box2D/Dynamics/b2Fixture.h"
#include "augs/math/vec2.h"
#include "augs/padding_byte.h"

enum class visibility_type {
	NONE,
};

struct visibility_information_request_input {
	augs::rgba color;

	b2Filter filter;
	padding_byte pad[2];

	float square_side = 0.f;
	float ignore_discontinuities_shorter_than = -1.f;

	vec2 offset;
};

struct line_of_sight_request_input {
	augs::rgba color;

	b2Filter obstruction_filter;
	b2Filter candidate_filter;
	float maximum_distance = 0.f;

	bool test_items = false;
	bool test_sentiences = false;
	bool test_attitudes = false;
	bool test_dangers = false;
};

namespace messages {
	struct visibility_information_request : message, visibility_information_request_input {
		visibility_type layer = visibility_type::NONE;
	};

	struct line_of_sight_request : message, line_of_sight_request_input {
		visibility_type layer = visibility_type::NONE;
	};

	struct visibility_information_response {
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
				const vec2 last_undiscovered_wall = vec2()) :
				points(points), winding(RIGHT),
				last_undiscovered_wall(last_undiscovered_wall), edge_index(0), is_boundary(false) {}
		};

		/* output */
		std::vector<edge> edges;

		/* first: edge index, second: location */
		std::vector<std::pair<int, vec2>> vertex_hits;
		std::vector<discontinuity> discontinuities;

		/* segments that denote narrow areas */
		std::vector<edge> marked_holes;

		discontinuity* get_discontinuity_for_edge(const size_t edge_num);
		discontinuity* get_discontinuity(const size_t disc_num);

		size_t get_num_discontinuities() const {
			return discontinuities.size();
		}

		size_t get_num_triangles() const;
		triangle get_triangle(const size_t index, const vec2 origin) const;
		std::vector<vec2> get_polygon(const float distance_epsilon, const vec2 expand_origin, const float expand_mult) const;
	};

	struct line_of_sight_response {
		std::set<entity_id> visible_items;
		std::set<entity_id> visible_sentiences;
		std::set<entity_id> visible_attitudes;
		std::set<entity_id> visible_dangers;

		bool sees(const entity_id) const;
	};
}