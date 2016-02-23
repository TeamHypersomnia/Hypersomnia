#pragma once
#include <string>
#include "../systems/physics_system.h"
#include "entity_system/entity_id.h"
#include "../globals/joints.h"
#include "../components/fixtures_component.h"

struct body_definition {
	int body_type = b2_dynamicBody;

	float angular_damping = 6.5f,
		linear_damping = 6.5f,
		gravity_scale = 0.f,
		angular_air_resistance = 0.f;

	bool fixed_rotation = false, bullet = false, angled_damping = false;

	vec2 velocity;
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
	bool is_friction_ground = false;

	void add_convex_polygon(const std::vector<vec2>&);
	void add_concave_polygon(const std::vector<vec2>&);

	void from_renderable(augs::entity_id);
};

components::physics& create_physics_component(body_definition, augs::entity_id subject);

components::fixtures& add_fixtures(fixture_definition, augs::entity_id subject);
components::fixtures& add_fixtures_to_other_body(fixture_definition, augs::entity_id fixtures_entity, augs::entity_id existing_body);

std::vector<b2Vec2> get_world_vertices(augs::entity_id subject, bool meters = true, int fixture_num = 0);

void create_weld_joint(augs::entity_id position_copyingd, augs::entity_id position_copyingr, vec2 orbit_offset, joint_name name = joint_name::JOINT);
void create_friction_joint(augs::entity_id inside_object, augs::entity_id friction_field, joint_name name = joint_name::JOINT);
void remove_joints_by_name(augs::entity_id, joint_name);
bool joint_exists(augs::entity_id, joint_name);
