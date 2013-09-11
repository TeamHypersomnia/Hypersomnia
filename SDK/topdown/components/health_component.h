#pragma once
#include "../components/render_component.h"
#include <Box2D\Dynamics\b2Fixture.h>

namespace components {
	struct health : public augmentations::entity_system::component {
		struct health_info {
			components::render death_render;
			float max_hp;

			bool should_disappear;
			float dead_lifetime_ms;
			b2Filter corpse_collision_filter;
			health_info() : should_disappear(false), dead_lifetime_ms(0.f), death_render(0, nullptr) {}

		} *info;
		
		float hp;
		bool dead;

		health(health_info* info, float hp) : info(info), hp(hp), dead(false) {}
	};
}