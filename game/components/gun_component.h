#pragma once
#include "augs/misc/timer.h"

#include "game/transcendental/entity_id.h"

#include "render_component.h"

#include "augs/misc/stepped_timing.h"
#include "augs/misc/recoil_player.h"
#include "augs/misc/minmax.h"

#include "padding_byte.h"

class gun_system;
class processing_system;

namespace components {
	struct gun  {
		enum class action_type : unsigned char {
			INVALID,

			SINGLE_SHOT,
			BOLT_ACTION,
			SEMI_AUTOMATIC,
			AUTOMATIC
		};

		augs::stepped_cooldown shot_cooldown = augs::stepped_cooldown(100);
		action_type action_mode = action_type::INVALID;
		unsigned short num_last_bullets_to_trigger_low_ammo_cue = 0;
		padding_byte pad;

		augs::minmax<float> muzzle_velocity;

		float damage_multiplier = 1.f;

		vec2 bullet_spawn_offset;

		float camera_shake_radius = 0.f;
		float camera_shake_spread_degrees = 0.f;

		int trigger_pressed = false;

		augs::minmax<float> shell_velocity;
		augs::minmax<float> shell_angular_velocity;

		float shell_spread_degrees = 20.f;

		recoil_player recoil;

		components::transform shell_spawn_offset;

		entity_id magic_missile_definition;

		float current_heat = 0.f;
		float gunshot_adds_heat = 0.05f;
		float maximum_heat = 1.f;
		float engine_sound_strength = 1.f;

		entity_id firing_engine_sound;
		entity_id muzzle_particles;

		template<class F>
		void for_each_held_id(F f) {
			f(magic_missile_definition);
			f(firing_engine_sound);
			f(muzzle_particles);
		}

		template<class F>
		void for_each_held_id(F f) const {
			f(magic_missile_definition);
			f(firing_engine_sound);
			f(muzzle_particles);
		}

		template <class Archive>
		void serialize(Archive& ar) {
			ar(
				CEREAL_NVP(action_mode),
				CEREAL_NVP(num_last_bullets_to_trigger_low_ammo_cue),

				CEREAL_NVP(muzzle_velocity),

				CEREAL_NVP(damage_multiplier),

				CEREAL_NVP(shot_cooldown),

				CEREAL_NVP(bullet_spawn_offset),

				CEREAL_NVP(camera_shake_radius),
				CEREAL_NVP(camera_shake_spread_degrees),

				CEREAL_NVP(trigger_pressed),

				CEREAL_NVP(shell_velocity),
				CEREAL_NVP(shell_angular_velocity),

				CEREAL_NVP(shell_spread_degrees),

				CEREAL_NVP(recoil),

				CEREAL_NVP(shell_spawn_offset),
				CEREAL_NVP(magic_missile_definition)
				);
		}

		vec2 calculate_muzzle_position(components::transform gun_transform) const;
		vec2 calculate_barrel_center(components::transform gun_transform) const;
	};
}