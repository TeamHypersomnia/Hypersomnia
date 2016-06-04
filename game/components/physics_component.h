#pragma once
#include "entity_system/entity_id.h"

#include "transform_component.h"

#include "math/vec2.h"
extern double METERS_TO_PIXELS;
extern double PIXELS_TO_METERS;
extern float METERS_TO_PIXELSf;
extern float PIXELS_TO_METERSf;

class b2Body;
#include <vector>
#include "misc/deterministic_timing.h"

namespace components {
	struct physics {
		static augs::entity_id get_owner_friction_field(augs::entity_id);
		static augs::entity_id get_owner_body_entity(augs::entity_id sub_entity);
		static bool is_entity_physical(augs::entity_id);
		static bool are_connected_by_friction(augs::entity_id child, augs::entity_id parent);
		static void set_active(augs::entity_id id, bool);
		static void resolve_density_of_associated_fixtures(augs::entity_id);

		b2Body* body = nullptr;
		
		augs::entity_id owner_friction_ground;
		std::vector<augs::entity_id> owner_friction_grounds;

		augs::deterministic_timeout since_dropped = augs::deterministic_timeout(0);


		bool enable_angle_motor = false;
		
		float target_angle = 0.f;
		float angle_motor_force_multiplier = 1.f;

		float measured_carried_mass = 0.f;

		/* a physically realistic alternative to max_speed variable, the bigger the value is, the lesser the maximum speed */
		float air_resistance = 2.0f;
		// -1.f - the same as the air resistance
		float angular_air_resistance = 0;

		std::vector<augs::entity_id> fixture_entities;

		void set_velocity(vec2);
		void set_linear_damping(float);
		void set_density(float);
		void set_linear_damping_vec(vec2);
		void apply_force(vec2);
		void apply_force(vec2, vec2 center_offset, bool wake = true);
		void apply_impulse(vec2);
		void apply_impulse(vec2, vec2 center_offset, bool wake = true);
		void apply_angular_impulse(float);
		
		vec2 velocity() const;
		float get_mass() const;
		float get_angle() const;
		vec2 get_position() const;
		vec2 get_mass_position() const;
		vec2 get_world_center() const;
		vec2 get_aabb_size() const;

		void set_transform(components::transform);
		void set_transform(augs::entity_id);
		
		augs::entity_id get_owner_friction_ground() const;
	};
}
