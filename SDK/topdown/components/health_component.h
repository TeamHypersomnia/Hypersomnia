#pragma once
#include "entity_system/entity.h"
#include "../renderable.h"

namespace components {
	struct health : public augmentations::entity_system::component {
		struct health_info {
			sprite death_sprite;
			float max_hp;

			bool should_disappear;
			float dead_lifetime_ms;
			health_info() : should_disappear(false), dead_lifetime_ms(0.f), death_sprite(nullptr) {}

		} *info;
		
		float hp;
		bool dead;

		health(health_info* info, float hp) : info(info), hp(hp), dead(false) {}
	};
}