#pragma once
#include <cstddef>
#include "augs/pad_bytes.h"

#include "augs/math/simple_physics.h"
#include "augs/misc/timing/stepped_timing.h"
#include "augs/misc/bound.h"
#include "augs/misc/enum/enum_boolset.h"

#include "game/enums/gun_action_type.h"

#include "game/cosmos/entity_id.h"
#include "game/cosmos/step_declaration.h"
#include "game/cosmos/entity_flavour_id.h"

#include "game/components/render_component.h"

#include "game/assets/ids/asset_ids.h"
#include "game/assets/recoil_player.h"

#include "game/detail/adversarial_meta.h"
#include "game/detail/view_input/sound_effect_input.h"
#include "game/detail/view_input/particle_effect_input.h"
#include "game/detail/sentience_shake.h"
#include "game/enums/weapon_action_type.h"
#include "augs/pad_bytes.h"

namespace augs {
	struct introspection_access;
}

enum class gun_special_state {
	// GEN INTROSPECTOR enum class gun_special_state
	NONE,
	AFTER_BURST
	// END GEN INTROSPECTOR
};

namespace components {
	struct gun {
		// GEN INTROSPECTOR struct components::gun
		real32 current_heat = 0.f;
		real32 chambering_progress_ms = 0.f;
		real32 max_heat_after_steam_schedule = 0.f;

		gun_special_state special_state = gun_special_state::NONE;

		bool steam_burst_scheduled = false;
		pad_bytes<1> pad;
		augs::enum_boolset<weapon_action_type, 1> just_pressed;

		augs::stepped_timestamp when_last_fired;
		augs::stepped_timestamp when_last_played_trigger_effect;
		augs::stepped_timestamp when_began_pulling_chambering_handle;

		recoil_player_instance recoil;

		simple_rot_vel magazine;
		// END GEN INTROSPECTOR
	};
}

namespace invariants {
	struct gun {
		// GEN INTROSPECTOR struct invariants::gun
		real32 shot_cooldown_ms = 100.f;

		gun_action_type action_mode = gun_action_type::INVALID;
		unsigned num_last_bullets_to_trigger_low_ammo_cue = 0;

		unsigned num_burst_bullets = 0;
		real32 burst_spread_degrees = 0.f;
		real32 burst_spread_degrees_variation = 0.f;
		real32 after_burst_chambering_ms = 1000.f;

		augs::bound<real32> muzzle_velocity = { 2000.f, 2000.f };

		real32 damage_multiplier = 1.f;

		augs::bound<real32> shell_velocity = { 300.f, 1700.f };
		augs::bound<real32> shell_angular_velocity = { 2.f, 14.f };

		real32 shell_spread_degrees = 20.f;

		real32 gunshot_adds_heat = 0.05f;
		real32 heat_cooldown_speed_mult = 1.f;
		real32 maximum_heat = 1.f;
		real32 minimum_heat_to_shoot = 0.f;

		real32 steam_burst_schedule_mult = 0.75f;
		real32 steam_burst_perform_diff = 0.5f;

		sound_effect_input heavy_heat_start_sound;
		sound_effect_input light_heat_start_sound;

		sound_effect_input steam_burst_sound;
		particle_effect_input steam_burst_particles;

		sound_effect_input chambering_sound;
		sound_effect_input muzzle_shot_sound;
		sound_effect_input low_ammo_cue_sound;

		sound_effect_input firing_engine_sound;
		sound_effect_input trigger_pull_sound;
		particle_effect_input firing_engine_particles;

		real32 kickback_towards_wielder = 0.f;
		real32 recoil_multiplier = 1.f;

		sentience_shake shot_shake;
		real32 shot_shake_radius = 0.f;

		bool allow_chambering_with_akimbo = false;
		bool allow_charge_in_chamber_magazine_when_chamber_loaded = true;
		pad_bytes<2> pad;

		constrained_entity_flavour_id<invariants::missile, components::sender> magic_missile_flavour;
		recoil_player_instance_def recoil;

		assets::plain_animation_id shoot_animation;
		adversarial_meta adversarial = { static_cast<money_type>(500) };
		// END GEN INTROSPECTOR

		auto get_steam_schedule_heat() const {
			return steam_burst_schedule_mult * maximum_heat;
		}

		auto get_steam_perform_diff() const {
			return steam_burst_perform_diff * maximum_heat;
		}
	};
}
