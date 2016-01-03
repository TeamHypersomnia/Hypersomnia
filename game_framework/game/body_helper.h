#pragma once
#include <string>
#include "../systems/physics_system.h"
#include "entity_system/entity_id.h"

namespace components {
	struct polygon;
}

namespace helpers {
	struct physics_info {
		enum {
			RECT,
			POLYGON,
			CIRCLE
		} type;
		
		std::vector<std::vector<vec2>> convex_polys;
		std::vector<vec2> original_model;
		vec2 rect_size;
		b2Filter filter;

		int body_type;

		float density, friction, restitution, angular_damping, linear_damping, radius, max_speed, gravity_scale;
		bool fixed_rotation, sensor, bullet, angled_damping;

		void add_convex(const std::vector<vec2> &);
		void add_concave(const std::vector<vec2> &);

		void from_renderable(const components::polygon&);

		physics_info();
	};

	//extern void create_physics_component(augs::entity_id subject, b2Filter filter, int = b2_dynamicBody);
	extern void create_physics_component(const physics_info&, augs::entity_id subject, int = b2_dynamicBody);
	extern std::vector<b2Vec2> get_transformed_shape_verts(augs::entity_id subject, bool meters = true);
	
	//template <typename T, typename TDef>
	//T* create_joint(world* owner, TDef* joint_def) {
	//	return static_cast<T*>(owner->get_system<physics_system>().b2world.CreateJoint(joint_def));
	//}

	extern augs::entity_id body_to_entity(b2Body*);
}