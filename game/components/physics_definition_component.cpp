#include "physics_definition_component.h"

namespace components {
	rects::ltrb<float> physics_definition::get_aabb_size() const {
		std::vector<vec2> all_points;
		
		for (auto& f : fixtures) {
			for(auto& c : f.convex_polys)
				all_points.insert(all_points.end(), c.begin(), c.end());

		}
		
		auto aabb = augs::get_aabb(all_points);
		
		return aabb; // vec2(aabb.w(), aabb.h());
	}
}