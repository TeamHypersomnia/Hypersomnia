#pragma once
#include <string>
#include "../systems/physics_system.h"
#include "entity_system/entity_id.h"
#include "../globals/joints.h"
#include "../components/fixtures_component.h"

namespace components {
	struct polygon;
}

namespace helpers {
	struct body_definition {
		int body_type = b2_dynamicBody;

		float angular_damping = 6.5f,
			linear_damping = 6.5f,
		gravity_scale = 0.f;

		bool fixed_rotation = false, bullet = false, angled_damping = false;
	};

	struct fixture_definition {
		void offset_vertices();

		enum {
			RECT,
			POLYGON,
			CIRCLE
		} type = RECT;
		
		std::vector<std::vector<vec2>> convex_polys;
		components::transform transform_vertices;

		vec2 rect_size;
		b2Filter filter;

		float density = 1.f, 
			friction = 0.f, 
			restitution = 0.f, 
			radius = 0.f;

		bool sensor = false;

		void add_convex_polygon(const std::vector<vec2>&);
		void add_concave_polygon(const std::vector<vec2>&);

		void from_renderable(augs::entity_id);
	};

	//extern void create_physics_component(augs::entity_id subject, b2Filter filter, int = b2_dynamicBody);

	extern components::physics& create_physics_component(body_definition, augs::entity_id subject);
	
	extern components::fixtures& add_fixtures(fixture_definition, augs::entity_id subject);
	extern components::fixtures& add_fixtures_to_other_body(fixture_definition, augs::entity_id fixtures_entity, augs::entity_id existing_body);

	extern std::vector<b2Vec2> get_world_vertices(augs::entity_id subject, bool meters = true, int fixture_num = 0);
	
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