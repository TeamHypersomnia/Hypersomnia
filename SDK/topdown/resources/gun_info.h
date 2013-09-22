#pragma once
#include <Box2D\Dynamics\b2Fixture.h>

namespace resources {
	struct render_info;

	struct gun_info {
		render_info* bullet_sprite;

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

		b2Filter bullet_collision_filter;

		gun_info() : max_rounds(0), bullets_once(0), spread_radians(0.f), bullet_min_damage(0.f), bullet_max_damage(0.f), is_automatic(false), bullet_sprite(nullptr),
			bullet_distance_offset(0.f), velocity_variation(0.f), shake_radius(0.f), shake_spread_radians(0.f),
			bullet_layer(0), max_bullet_distance(1000.f) {
				bullet_collision_filter.groupIndex = -1;
		}
	};
}
