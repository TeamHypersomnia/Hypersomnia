#pragma once
#include "padding_byte.h"

#include "augs/audio/sound_effect_modifier.h"

#include "augs/misc/timer.h"
#include "augs/misc/stepped_timing.h"
#include "augs/misc/recoil_player.h"
#include "augs/misc/minmax.h"

#include "game/enums/gun_action_type.h"
#include "game/transcendental/entity_id.h"
#include "game/transcendental/step_declaration.h"
#include "game/assets/sound_buffer_id.h"
#include "game/components/render_component.h"

namespace augs {
	struct introspection_access;
}

namespace components {
	struct gun  {
		// GEN INTROSPECTOR struct components::gun
		augs::stepped_cooldown shot_cooldown = augs::stepped_cooldown(100);
		gun_action_type action_mode = gun_action_type::INVALID;
		unsigned short num_last_bullets_to_trigger_low_ammo_cue = 0;
		bool is_trigger_pressed = false;

		friend struct augs::introspection_access;

	private:
		bool is_cocking_handle_being_pulled = false;
		std::array<padding_byte, 3> pad;
		
		augs::stepped_timestamp when_began_pulling_cocking_handle;
	public:
		float cocking_handle_pull_duration_ms = 500.f;

		void set_cocking_handle_pulling(
			const bool enabled, 
			const augs::stepped_timestamp now
		);

		static void load_next_round(
			const entity_id subject,
			const logic_step step
		);

		augs::minmax<float> muzzle_velocity;

		float damage_multiplier = 1.f;

		vec2 bullet_spawn_offset;

		float camera_shake_radius = 0.f;
		float camera_shake_spread_degrees = 0.f;

		augs::minmax<float> shell_velocity;
		augs::minmax<float> shell_angular_velocity;

		float shell_spread_degrees = 20.f;

		recoil_player recoil;

		components::transform shell_spawn_offset;

		child_entity_id magic_missile_definition;

		float current_heat = 0.f;
		float gunshot_adds_heat = 0.05f;
		float maximum_heat = 1.f;
		float engine_sound_strength = 1.f;

		child_entity_id firing_engine_sound;
		child_entity_id muzzle_particles;

		sound_response muzzle_shot_sound_response;
		// END GEN INTROSPECTOR

		vec2 calculate_muzzle_position(components::transform gun_transform) const;
		vec2 calculate_barrel_center(components::transform gun_transform) const;
	};
}