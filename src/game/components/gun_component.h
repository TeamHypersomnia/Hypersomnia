#pragma once
#include <cstddef>
#include "augs/pad_bytes.h"

#include "augs/audio/sound_effect_modifier.h"

#include "augs/misc/timer.h"
#include "augs/misc/stepped_timing.h"
#include "augs/misc/minmax.h"

#include "game/enums/gun_action_type.h"
#include "game/transcendental/entity_id.h"
#include "game/transcendental/step_declaration.h"
#include "game/assets/sound_buffer_id.h"
#include "game/components/render_component.h"
#include "game/assets/recoil_player.h"
#include "game/assets/recoil_player_id.h"

namespace augs {
	struct introspection_access;
}

struct recoil_player_instance {
	// GEN INTROSPECTOR struct recoil_player_instance
	assets::recoil_player_id id = assets::recoil_player_id::INVALID;
	std::size_t index = 0;
	// END GEN INTROSPECTOR

	vec2 shoot_and_get_impulse(const recoil_player& meta) {
		const auto result = meta.offsets[index++];
		index %= meta.offsets.size();
		return result;
	}
};

namespace components {
	struct gun  {
		// GEN INTROSPECTOR struct components::gun
		augs::stepped_cooldown shot_cooldown = augs::stepped_cooldown(100);
		unsigned short num_last_bullets_to_trigger_low_ammo_cue = 0;
		gun_action_type action_mode = gun_action_type::INVALID;
		bool is_trigger_pressed = false;

		augs::minmax<float> muzzle_velocity;

		float damage_multiplier = 1.f;

		vec2 bullet_spawn_offset;

		float camera_shake_radius = 0.f;
		float camera_shake_spread_degrees = 0.f;

		augs::minmax<float> shell_velocity;
		augs::minmax<float> shell_angular_velocity;

		float shell_spread_degrees = 20.f;

		components::transform shell_spawn_offset;

		child_entity_id magic_missile_definition;

		float current_heat = 0.f;
		float gunshot_adds_heat = 0.05f;
		float maximum_heat = 1.f;
		float engine_sound_strength = 1.f;

		child_entity_id firing_engine_sound;
		child_entity_id muzzle_particles;

		sound_effect_input muzzle_shot_sound;
		sound_effect_input low_ammo_cue_sound;

	private:
		friend struct augs::introspection_access;

		bool is_cocking_handle_being_pulled = false;
		pad_bytes<3> pad;

		augs::stepped_timestamp when_began_pulling_cocking_handle;
	public:
		float cocking_handle_pull_duration_ms = 500.f;

		recoil_player_instance recoil;
		// END GEN INTROSPECTOR

		static void load_next_round(
			const entity_id subject,
			const logic_step step
		);

		void set_cocking_handle_pulling(
			const bool enabled,
			const augs::stepped_timestamp now
		);

		vec2 calculate_muzzle_position(components::transform gun_transform) const;
		vec2 calculate_barrel_center(components::transform gun_transform) const;
	};
}