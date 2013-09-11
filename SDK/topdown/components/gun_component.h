#pragma once
#include "utility/timer.h"
#include "entity_system/component.h"
#include "entity_system/entity_ptr.h"
#include <Box2D\Dynamics\b2Fixture.h>

struct renderable;
namespace components {
	struct gun : public augmentations::entity_system::component {
		struct gun_info {
			renderable* bullet_sprite;

			unsigned max_rounds;

			unsigned bullets_once;
			float spread_radians;
			float bullet_min_damage;
			float bullet_max_damage;
			float bullet_speed;
			float shooting_interval_ms;
			float velocity_variation;
			float max_bullet_distance;

			float bullet_distance_offset;
			float shake_radius;
			float shake_spread_radians;

			bool is_automatic;

			unsigned bullet_layer;

			/* detail - usually you don't want to change it */
			b2Filter bullet_collision_filter;

			gun_info() : max_rounds(0), bullets_once(0), spread_radians(0.f), bullet_min_damage(0.f), bullet_max_damage(0.f), is_automatic(false), bullet_sprite(nullptr),
				bullet_distance_offset(0.f), velocity_variation(0.f), shake_radius(0.f), shake_spread_radians(0.f),
				bullet_layer(0), max_bullet_distance(1000.f) {
					bullet_collision_filter.groupIndex = -1;
			}
		} *info;
		unsigned current_rounds;

		bool reloading, trigger;

		augmentations::util::timer shooting_timer;
		augmentations::entity_system::entity_ptr target_camera_shake;

		gun(gun_info* info)
			: info(info), current_rounds(0), 
			reloading(false), trigger(false), target_camera_shake(nullptr) {}
	};
}