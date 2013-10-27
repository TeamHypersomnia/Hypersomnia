#pragma once
#include <string>

namespace augmentations {
	namespace entity_system {
		class entity;
	}
}

namespace resources {
	class polygon;
}

namespace topdown {
	struct physics_info {
		enum {
			RECT,
			POLYGON
		} type;
		
		std::vector<std::vector<augmentations::vec2<>>> convex_polys;
		augmentations::vec2<> rect_size;
		b2Filter filter;

		float density, friction, angular_damping, linear_damping;
		bool fixed_rotation, sensor;

		void add_convex(const std::vector < augmentations::vec2 < >> &);
		void add_concave(const std::vector < augmentations::vec2 < >> &);

		void from_renderable(const resources::polygon&);

		physics_info();
	};

	//extern void create_physics_component(augmentations::entity_system::entity& subject, b2Filter filter, int = b2_dynamicBody);
	extern void create_physics_component(const physics_info&, augmentations::entity_system::entity& subject, int = b2_dynamicBody);
}