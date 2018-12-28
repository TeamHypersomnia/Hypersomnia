#pragma once
#include "message.h"
#include <vector>
#include "augs/graphics/rgba.h"
#include "augs/misc/simple_pair.h"
#include "game/components/transform_component.h"
#include "3rdparty/Box2D/Dynamics/b2Fixture.h"
#include "augs/math/vec2.h"
#include "augs/pad_bytes.h"

struct visibility_information_request_input {
	b2Filter filter;
	pad_bytes<2> pad;

	vec2 queried_rect;
	float ignore_discontinuities_shorter_than = -1.f;

	vec2 offset;
};

namespace messages {
	struct visibility_information_request : message, visibility_information_request_input {
		static auto empty() {
			return visibility_information_request();
		}

		transformr eye_transform;
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

		vec2 source_queried_rect;

		/* output */
		std::vector<edge> edges;

		/* first: edge index, second: location */
		std::vector<augs::simple_pair<int, vec2>> vertex_hits;
		std::vector<discontinuity> discontinuities;

		/* segments that denote narrow areas */
		std::vector<edge> marked_holes;

		void clear();

		bool empty() const {
			return edges.empty() && vertex_hits.empty() && discontinuities.empty() && marked_holes.empty();
		}

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
}