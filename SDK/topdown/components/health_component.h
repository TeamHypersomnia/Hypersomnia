#pragma once
#include "../components/render_component.h"
#include "../game/body_helper.h"

namespace components {
	struct health : public augmentations::entity_system::component {
		components::render corpse_render;
		topdown::physics_info corpse_body;

		float max_hp;

		bool should_disappear;
		float dead_lifetime_ms;
		
		float hp;
		bool dead;

		health() : should_disappear(false), dead_lifetime_ms(0.f), hp(0.f), dead(false) {}
	};
}