#pragma once
#include "message.h"
#include <unordered_set>
#include <vector>
#include "augs/graphics/rgba.h"
#include "augs/misc/simple_pair.h"
#include "game/components/transform_component.h"
#include "3rdparty/Box2D/Dynamics/b2Fixture.h"
#include "augs/math/vec2.h"
#include "augs/pad_bytes.h"

enum class visibility_type {
	NONE,
};

struct visibility_information_request_input {
	b2Filter filter;
	pad_bytes<2> pad;

	float square_side = 0.f;
	float ignore_discontinuities_shorter_than = -1.f;

	vec2 offset;
};

struct line_of_sight_request_input {
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
		transformr eye_transform;
		visibility_type layer = visibility_type::NONE;
	};

	struct line_of_sight_request : message, line_of_sight_request_input {
		visibility_type layer = visibility_type::NONE;
	};

	struct visibility_information_response {
		using edge = augs::simple_pair<vec2, vec2>;
		using triangle = std::array<vec2, 3>;

		struct discontinuity {
			int edge_index = 0;
			bool is_boundary = false;
			vec2 normal;

			edge points;
			vec2 last_undiscovered_wall;

			enum {
				RIGHT,
				LEFT
			} winding = RIGHT;

			discontinuity(
				const edge& points = edge(),
				const vec2 last_undiscovered_wall = vec2()
			) :
				points(points), 
				last_undiscovered_wall(last_undiscovered_wall)
			{}
		};

		float source_square_side = 0.f;

		/* output */
		std::vector<edge> edges;

		/* first: edge index, second: location */
		std::vector<augs::simple_pair<int, vec2>> vertex_hits;
		std::vector<discontinuity> discontinuities;

		/* segments that denote narrow areas */
		std::vector<edge> marked_holes;

		discontinuity* get_discontinuity_for_edge(const size_t edge_num);
		discontinuity* get_discontinuity(const size_t disc_num);
		const discontinuity* get_discontinuity_for_edge(const size_t edge_num) const;
		const discontinuity* get_discontinuity(const size_t disc_num) const;

		size_t get_num_discontinuities() const {
			return discontinuities.size();
		}

		size_t get_num_triangles() const;

		triangle get_world_triangle(const size_t index, const vec2 origin) const;
		std::vector<vec2> get_world_polygon(const float distance_epsilon, const vec2 expand_origin, const float expand_mult) const;
	};

	struct line_of_sight_response {
		std::unordered_set<entity_id> visible_items;
		std::unordered_set<entity_id> visible_sentiences;
		std::unordered_set<entity_id> visible_attitudes;
		std::unordered_set<entity_id> visible_dangers;

		bool sees(const entity_id) const;
	};
}