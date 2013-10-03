#pragma once
#include "../components/render_component.h"

namespace components {
	struct health : public augmentations::entity_system::component {
		components::render death_render;
		float max_hp;

		bool should_disappear;
		float dead_lifetime_ms;
		b2Filter corpse_collision_filter;
		
		float hp;
		bool dead;

		health() : should_disappear(false), dead_lifetime_ms(0.f), hp(0.f), dead(false) {}
	};
}