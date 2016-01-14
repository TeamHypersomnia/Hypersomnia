#pragma once
#include <string>
#include "../systems/physics_system.h"
#include "entity_system/entity_id.h"
#include "../globals/joints.h"

namespace components {
	struct polygon;
}

namespace helpers {
	struct physics_info {
		enum {
			RECT,
			POLYGON,
			CIRCLE
		} type = RECT;
		
		std::vector<std::vector<vec2>> convex_polys;
		std::vector<vec2> original_model;
		
		vec2 rect_size;
		b2Filter filter;

		int body_type = b2_dynamicBody;

		float density = 1.f, 
			friction = 0.f, 
			restitution = 0.f, 
			angular_damping = 0.f, 
			linear_damping = 0.f, 
			radius = 0.f, 
			gravity_scale = 0.f;

		bool fixed_rotation = false, sensor = false, bullet = false, angled_damping = false;

		void add_convex(const std::vector<vec2> &);
		void add_concave(const std::vector<vec2> &);

		void from_renderable(augs::entity_id);
	};

	//extern void create_physics_component(augs::entity_id subject, b2Filter filter, int = b2_dynamicBody);
	extern components::physics& create_physics_component(const physics_info&, augs::entity_id subject, int = b2_dynamicBody);
	extern std::vector<b2Vec2> get_transformed_shape_verts(augs::entity_id subject, bool meters = true);
	
	void create_weld_joint(augs::entity_id chased, augs::entity_id chaser, vec2 orbit_offset, joint_name name = joint_name::JOINT);
	void create_friction_joint(augs::entity_id inside_object, augs::entity_id friction_field, joint_name name = joint_name::JOINT);
	void remove_joints(augs::entity_id, joint_name);
	bool joint_exists(augs::entity_id, joint_name);

	//template <typename T, typename TDef>
	//T* create_joint(world* owner, TDef* joint_def) {
	//	return static_cast<T*>(owner->get_system<physics_system>().b2world.CreateJoint(joint_def));
	//}

	extern augs::entity_id body_to_entity(b2Body*);
}