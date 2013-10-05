#pragma once
#include <string>

namespace augmentations {
	namespace entity_system {
		class entity;
	}
}

namespace topdown {
	extern b2World* current_b2world;

	struct physics_info {
		enum {
			RECT,
			POLYGON
		} type;
		
		std::vector<augmentations::vec2<>> vertices;
		augmentations::vec2<> rect_size;
		b2Filter filter;

		float density, angular_damping, linear_damping;
		bool fixed_rotation;

		void add_vertex(augmentations::vec2<> v) {
			vertices.push_back(v);
		}

		physics_info();
	};

	//extern void create_physics_component(augmentations::entity_system::entity& subject, b2Filter filter, int = b2_dynamicBody);
	extern void create_physics_component(const physics_info&, augmentations::entity_system::entity& subject, int = b2_dynamicBody);
}