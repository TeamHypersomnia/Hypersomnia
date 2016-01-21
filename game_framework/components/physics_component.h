#pragma once
#include "entity_system/component.h"
#include "entity_system/entity_id.h"

#include "transform_component.h"

#include "math/vec2.h"
extern double METERS_TO_PIXELS;
extern double PIXELS_TO_METERS;
extern float METERS_TO_PIXELSf;
extern float PIXELS_TO_METERSf;

class b2Body;
#include <vector>

namespace components {
	struct physics : public augs::component {
		static physics& get_owner_body(augs::entity_id);
		static augs::entity_id get_owner_body_entity(augs::entity_id);
		static bool is_physical(augs::entity_id);
		static bool are_connected_by_friction(augs::entity_id child, augs::entity_id parent);

		b2Body* body = nullptr;
		
		augs::entity_id owner_friction_ground;
		std::vector<augs::entity_id> owner_friction_grounds;

		augs::entity_id get_owner_friction_ground();

		bool enable_angle_motor = false;
		float target_angle = 0.f;
		float angle_motor_force_multiplier = 1.f;

		float measured_carried_mass = 0.f;

		/* a physically realistic alternative to max_speed variable, the bigger the value is, the lesser the maximum speed */
		float air_resistance = 2.0f;
		// -1.f - the same as the air resistance
		float angular_air_resistance = 0;

		std::vector<augs::entity_id> fixture_entities;

		vec2 velocity();
		void set_velocity(vec2);
		void set_linear_damping(float);
		void set_linear_damping_vec(vec2);
		void apply_force(vec2);
		void apply_force(vec2, vec2 center_offset, bool wake = true);
		void apply_impulse(vec2);
		void apply_impulse(vec2, vec2 center_offset, bool wake = true);
		float get_mass();
		vec2 get_position();
		vec2 get_world_center();

		void set_transform(components::transform);
		void set_transform(augs::entity_id);
	};
}
