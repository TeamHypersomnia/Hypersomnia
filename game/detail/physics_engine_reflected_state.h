#pragma once
#include <vector>
#include <array>
#include <Box2D\Dynamics\b2Fixture.h>

#include "misc/stepped_timing.h"

#include "game/entity_id.h"
#include "game/components/transform_component.h"
#include "game/detail/convex_partitioned_shape.h"

class b2Body;
class b2Fixture;
class physics_system;

struct rigid_body_black_box_detail {
	physics_system* parent_system = nullptr;
	b2Body* body = nullptr;
	entity_id body_owner;
	std::vector<entity_id> fixture_entities;
};

struct rigid_body_black_box {
	components::transform transform;

	bool activated = true;

	enum class type {
		DYNAMIC,
		STATIC,
		KINEMATIC
	};

	type body_type = type::DYNAMIC;

	float angular_damping = 6.5f;
	float linear_damping = 6.5f;
	vec2 linear_damping_vec;
	float gravity_scale = 0.f;

	bool fixed_rotation = false;
	bool bullet = false;
	bool angled_damping = false;

	vec2 velocity;
	float angular_velocity = 0.f;
};

struct rigid_body_white_box {
	entity_id owner_friction_ground;
	std::vector<entity_id> owner_friction_grounds;

	augs::stepped_timeout since_dropped;

	bool enable_angle_motor = false;

	float target_angle = 0.f;
	float angle_motor_force_multiplier = 1.f;

	float measured_carried_mass = 0.f;

	/* a physically realistic alternative to max_speed variable, the bigger the value is, the lesser the maximum speed */
	float air_resistance = 2.0f;
	// -1.f - the same as the air resistance
	float angular_air_resistance = 0;
};

struct rigid_body_definition : public rigid_body_black_box, public rigid_body_white_box {

};

struct colliders_black_box_detail {
	entity_id all_fixtures_owner;
	physics_system* parent_system = nullptr;
	std::vector<std::vector<b2Fixture*>> fixtures_per_collider;
};

struct convex_partitioned_collider {
	convex_partitioned_shape shape;
	b2Filter filter;

	float density = 1.f;
	float density_multiplier = 1.f;
	float friction = 0.f;
	float restitution = 0.f;

	bool sensor = false;
};

struct colliders_black_box {
	std::vector<convex_partitioned_collider> colliders;
	bool activated = true;

	entity_id owner_body;

	enum class offset_type {
		ITEM_ATTACHMENT_DISPLACEMENT,
		SPECIAL_MOVE_DISPLACEMENT
	};

	std::array<components::transform, 2> offsets_for_created_shapes;

	convex_partitioned_collider& new_collider() {
		colliders.push_back(convex_partitioned_collider());
		return *colliders.rbegin();
	}
};

struct colliders_white_box {
	bool is_friction_ground = false;
	bool disable_standard_collision_resolution = false;
};

struct colliders_definition : public colliders_black_box, public colliders_white_box {

};
