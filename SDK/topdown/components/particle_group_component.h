#pragma once
#include <vector>
#include "entity_system/component.h"
#include "math/vec2d.h"
#include "../renderable.h"

class physics_system;
namespace components {
	/* hack - the only component that has logic */
	struct particle_group : public augmentations::entity_system::component, public renderable {
		physics_system* optional_physics;
		
		struct particle {
			augmentations::vec2<> pos, vel, acc;
			sprite face;
			float rotation;
			float rotation_speed;
			float linear_damping;
			float angular_damping;
			float lifetime_ms, max_lifetime_ms;
			bool should_disappear;
			particle() : face(nullptr), lifetime_ms(0.f), should_disappear(true), rotation(0.f), rotation_speed(0.f) {}
		};

		struct emission {
			enum class type {
				BURST,
				STREAM
			} type;

			float spread_radians;
			float velocity_min;
			float velocity_max;
			float angular_velocity_min;
			float angular_velocity_max;
			float particles_per_sec_min;
			float particles_per_sec_max;
			float stream_duration_ms_min;
			float stream_duration_ms_max;
			float particle_lifetime_ms_min;
			float particle_lifetime_ms_max;
			float size_multiplier_min;
			float size_multiplier_max;
			float initial_rotation_variation;
			unsigned particles_per_burst_min;
			unsigned particles_per_burst_max;

			bool randomize_acceleration;
			float acc_min;
			float acc_max;

			augmentations::vec2<> offset;
			float angular_offset;

			std::vector<particle_group::particle> particle_templates;
			unsigned particle_group_layer;

			emission() : acc_min(0.f), acc_max(0.f), randomize_acceleration(false) {}
		};

		emission* stream_info;

		float stream_lifetime_ms;
		float stream_max_lifetime_ms;
		/* detail */
		float stream_particles_to_spawn;

		std::vector<particle> particles;
		/* detail, only used by subject stream to indicate that it will no longer emit particles */
		bool destroy_when_empty;

		particle_group() 
			: optional_physics(nullptr), destroy_when_empty(true), stream_info(nullptr), stream_lifetime_ms(0.f), stream_particles_to_spawn(0.f) {}

		virtual void draw(buffer&, const components::transform&, vec2<> camera_pos) override;
		virtual bool is_visible(rects::xywh visibility_aabb, const components::transform&) override;
		virtual b2Body* create_body(entity_system::entity& subject, b2World& b2world, b2BodyType type) override;
	};
}