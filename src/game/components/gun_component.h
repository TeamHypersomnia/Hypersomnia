#pragma once
#include <cstddef>
#include "augs/pad_bytes.h"

#include "augs/math/simple_physics.h"
#include "augs/misc/timing/stepped_timing.h"
#include "augs/misc/minmax.h"

#include "game/enums/gun_action_type.h"

#include "game/transcendental/entity_id.h"
#include "game/transcendental/step_declaration.h"
#include "game/transcendental/entity_flavour_id.h"

#include "game/components/render_component.h"

#include "game/assets/ids/asset_ids.h"
#include "game/assets/recoil_player.h"

#include "game/detail/view_input/sound_effect_input.h"
#include "game/detail/view_input/particle_effect_input.h"

namespace augs {
	struct introspection_access;
}

namespace components {
	struct gun {
		// GEN INTROSPECTOR struct components::gun
		augs::stepped_timestamp when_last_fired;
		augs::stepped_timestamp when_last_played_trigger_effect;

		bool is_trigger_pressed = false;
		bool is_cocking_handle_being_pulled = false;
		bool steam_burst_scheduled = false;
		bool play_trigger_effect_once = false;

		float current_heat = 0.f;
		float max_heat_after_steam_schedule = 0.f;

		augs::stepped_timestamp when_began_pulling_cocking_handle;

		recoil_player_instance recoil;

		simple_rot_vel magazine;
		// END GEN INTROSPECTOR

		void set_cocking_handle_pulling(
			const bool enabled,
			const augs::stepped_timestamp now
		);
	};
}

namespace invariants {
	struct gun {
		// GEN INTROSPECTOR struct invariants::gun
		float shot_cooldown_ms = 100.f;

		gun_action_type action_mode = gun_action_type::INVALID;
		unsigned num_last_bullets_to_trigger_low_ammo_cue = 0;

		augs::minmax<float> muzzle_velocity = { 2000.f, 2000.f };

		float damage_multiplier = 1.f;

		augs::minmax<float> shell_velocity = { 300.f, 1700.f };
		augs::minmax<float> shell_angular_velocity = { 2.f, 14.f };

		float shell_spread_degrees = 20.f;

		transformr shell_spawn_offset;

		float gunshot_adds_heat = 0.05f;
		float heat_cooldown_speed_mult = 1.f;
		float maximum_heat = 1.f;
		float minimum_heat_to_shoot = 0.f;

		float steam_burst_schedule_mult = 0.75f;
		float steam_burst_perform_diff = 0.5f;

		sound_effect_input heavy_heat_start_sound;
		sound_effect_input light_heat_start_sound;

		sound_effect_input steam_burst_sound;
		particle_effect_input steam_burst_particles;

		sound_effect_input muzzle_shot_sound;
		sound_effect_input low_ammo_cue_sound;

		sound_effect_input firing_engine_sound;
		particle_effect_input firing_engine_particles;

		real32 kickback_towards_wielder = 0.f;
		real32 recoil_multiplier = 1.f;

		float cocking_handle_pull_duration_ms = 500.f;

		constrained_entity_flavour_id<invariants::missile, components::sender> magic_missile_flavour;
		recoil_player_instance_def recoil;

		assets::plain_animation_id shoot_animation;
		// END GEN INTROSPECTOR

		auto get_steam_schedule_heat() const {
			return steam_burst_schedule_mult * maximum_heat;
		}

		auto get_steam_perform_diff() const {
			return steam_burst_perform_diff * maximum_heat;
		}
	};
}
