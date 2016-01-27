#pragma once

#include "misc/timer.h"

#include "entity_system/entity.h"

#include "render_component.h"
#include "../game/body_helper.h"

class gun_system;
namespace components {
	struct gun  {
		enum state {
			READY,
			SWINGING,
			SHOOTING_INTERVAL,
			SWINGING_INTERVAL
		};
		
		enum trigger {
			NONE,
			MELEE,
			SHOOT
		};

		int current_state, trigger_mode;

		components::render bullet_render;
		helpers::fixture_definition bullet_body;
		b2Filter melee_filter;
		b2Filter melee_obstruction_filter;

		unsigned max_rounds;

		unsigned bullets_once;
		float spread_degrees;
		std::pair<float, float> bullet_damage;
		std::pair<float, float> bullet_speed;
		float shooting_interval_ms;
		float max_bullet_distance;

		vec2 bullet_distance_offset;
		float shake_radius;
		float shake_spread_degrees;

		bool is_automatic;

		unsigned current_rounds;

		bool current_swing_direction;

		float swing_duration;
		float swing_radius;
		float swing_angle;
		float swing_angular_offset;
		float swing_interval_ms;
		int query_vertices;

		void set_bullet_filter(b2Filter f) {
			bullet_body.filter = f;
		}

		void shake_camera(float direction);

		struct uncopyable {
			augs::entity_id target_barrel_smoke_group;
			uncopyable& operator=(const uncopyable& b) { return *this; }
		} barrel_smoke;

		augs::entity_id& get_barrel_smoke() {
			return barrel_smoke.target_barrel_smoke_group;
		}

		void transfer_barrel_smoke(augs::entity_id another, bool overwrite_comps);

		augs::entity_id target_camera_to_shake;

		gun()
			: max_rounds(0), bullets_once(0), spread_degrees(0.f), bullet_damage(std::make_pair(0.f, 0.f)), is_automatic(false),
			bullet_distance_offset(0.f), shake_radius(0.f), shake_spread_degrees(0.f), max_bullet_distance(1000.f), current_rounds(0),
			 swing_radius(0.f), swing_angle(0.f), swing_angular_offset(0.f), query_vertices(7), swing_duration(0.f),
			 trigger_mode(NONE), current_state(READY), bullet_speed(std::make_pair(0.f, 0.f)), current_swing_direction(false) {
				bullet_body.filter.groupIndex = -1;
		}

		bool outdated(float limit_ms) {
			return limit_ms < state_timer.get<std::chrono::milliseconds>();
		}

		/* for now, if somebody is a stuntman let him shoot faster than the firerate by dropping and picking up the weapon */
		void drop_logic() {
			trigger_mode = NONE;
			current_state = READY;
		}

	private:
		friend class gun_system;

		augs::timer state_timer;
	};
}