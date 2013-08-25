#pragma once
#include <vector>
#include "entity_system/component.h"
#include "math/vec2d.h"
#include "../renderable.h"

struct physics_system;
namespace components {
	struct particle_group : public augmentations::entity_system::component {
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
		
		std::vector<particle> particles;
		particle_group() : optional_physics(nullptr) {}
	};
}