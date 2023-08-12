#pragma once
#include <optional>

#include "augs/graphics/rgba.h"

#include "augs/misc/timing/stepped_timing.h"
#include "augs/misc/enum/enum_map.h"
#include "augs/misc/value_meter.h"

#include "augs/templates/type_list.h"
#include "augs/templates/constexpr_arithmetic.h"
#include "augs/templates/always_false.h"
#include "augs/misc/constant_size_vector.h"

#include "game/assets/ids/asset_ids.h"

#include "game/cosmos/entity_id.h"

#include "game/components/transform_component.h"

#include "game/detail/all_sentience_meters.h"
#include "augs/math/physics_structs.h"
#include "game/detail/spells/all_spells.h"
#include "game/detail/perks/all_perks.h"
#include "game/detail/sentience_shake.h"

#include "game/detail/view_input/sound_effect_input.h"
#include "game/detail/view_input/particle_effect_input.h"
#include "game/detail/damage_origin.h"

#include "game/detail/sentience/detached_body_parts.h"
#include "game/enums/interaction_result_type.h"
#include "game/enums/weapon_action_type.h"
#include "game/detail/inventory/hand_count.h"
#include "game/components/explosive_component.h"
#include "augs/pad_bytes.h"

struct damage_owner {
	// GEN INTROSPECTOR struct damage_owner
	signi_entity_id who;
	int hits = 1;
	meter_value_type applied_damage = 0;
	meter_value_type hp_loss = 0;
	meter_value_type pe_loss = 0;
	// END GEN INTROSPECTOR

	bool operator<(const damage_owner& b) const {
		/* Bigger come first */
		return applied_damage > b.applied_damage;
	}
};

using damage_owners_vector = augs::constant_size_vector<damage_owner, 4>;

namespace components {
	struct sentience {
		// GEN INTROSPECTOR struct components::sentience
		augs::stepped_timestamp time_of_last_received_damage;
		augs::stepped_timestamp time_of_last_exertion;

		augs::stepped_cooldown cast_cooldown_for_all_spells;
		augs::stepped_cooldown spawn_protection_cooldown;

		meter_instance_tuple meters;

		learnt_spells_array_type learnt_spells = {};

		spell_instance_tuple spells;
		perk_instance_tuple perks;

		spell_id currently_casted_spell;
		
		transformr transform_when_spell_casted;
		augs::stepped_timestamp time_of_last_spell_cast;
		augs::stepped_timestamp time_of_last_exhausted_cast;

		augs::stepped_timestamp time_of_last_shake;
		augs::stepped_timestamp when_knocked_out;
		augs::stepped_timestamp when_corpse_catched_fire;
		sentience_shake shake = sentience_shake::zero();

		bool is_requesting_interaction = false;
		bool spells_drain_pe = true;
		pad_bytes<2> pad;
		interaction_result_type last_interaction_result = interaction_result_type::NOTHING_FOUND;

		damage_owners_vector damage_owners;
		damage_origin knockout_origin;

		detached_body_parts detached;

		std::array<bool, hand_count_v> hand_flags = {};
		bool block_flag = false;
		bool has_exploded = false;

		std::array<augs::stepped_timestamp, hand_count_v> when_hand_pressed = {};

		real32 rotation_inertia_ms = 0.1f;

		real32 audio_flash_secs = 0.f;
		real32 visual_flash_secs = 0.f;
		rgba last_assigned_color = rgba::zero;

		transformr transform_when_danger_caused;
		augs::stepped_timestamp time_of_last_caused_danger;
		real32 radius_of_last_caused_danger = 0.f;
		// END GEN INTROSPECTOR

		bool is_interacting() const {
			const bool just_begun = last_interaction_result == interaction_result_type::CAN_BEGIN;
			const bool in_progress = last_interaction_result == interaction_result_type::IN_PROGRESS;

			return just_begun || in_progress;
		}

		bool is_learnt(const spell_id id) const {
			ensure(id.is_set());
			return learnt_spells[id.get_index()] == true;
		}

		bool is_spell_being_cast() const {
			return currently_casted_spell.is_set();
		}

		rgba calc_health_color(const float time_pulse_multiplier) const;
		std::optional<rgba> find_low_health_border(const unsigned timestamp_ms) const;

		bool is_conscious() const;
		bool unconscious_but_alive() const;
		bool is_dead() const;

		template <class T>
		auto& get() {
			if constexpr(is_one_of_list_v<T, decltype(meters)>) {
				return std::get<T>(meters);
			}
			else if constexpr(is_one_of_list_v<T, decltype(perks)>) {
				return std::get<T>(perks);
			}
			else {
				static_assert(always_false_v<T>);
			}
		}

		template <class T>
		const auto& get() const {
			if constexpr(is_one_of_list_v<T, decltype(meters)>) {
				return std::get<T>(meters);
			}
			else if constexpr(is_one_of_list_v<T, decltype(perks)>) {
				return std::get<T>(perks);
			}
			else {
				static_assert(always_false_v<T>);
			}
		}
	};
}

using remnant_flavour_id = constrained_entity_flavour_id<invariants::remnant>;

struct corpse_remnant_def {
	// GEN INTROSPECTOR struct corpse_remnant_def
	vec2 offset;
	remnant_flavour_id flavour_id;
	// END GEN INTROSPECTOR
};

using corpse_remnant_flavour_vector = augs::constant_size_vector<corpse_remnant_def, 5>;

namespace invariants {
	struct sentience {
		// GEN INTROSPECTOR struct invariants::sentience
		sentience_shake_settings shake_settings;
		real32 dash_impulse_mult = 1.f;

		real32 comfort_zone = 500.f;
		real32 minimum_danger_amount_to_evade = 20.f;
		real32 danger_amount_from_hostile_attitude = 100.f;

		real32 interaction_hitbox_radius = 100.f;
		real32 interaction_hitbox_range = 90.f;

		real32 aimpunch_mult = 1.f;
		real32 const_inertia_damage_ratio = 2.f;
		real32 linear_inertia_damage_ratio = 1.f;

		impulse_mults knockout_impulse = { 1000.f, 80.f };

		sound_effect_input health_decrease_sound;
		sound_effect_input corpse_health_decrease_sound;

		sound_effect_input consciousness_decrease_sound;
		sound_effect_input headshot_sound;

		sound_effect_input death_sound;
		sound_effect_input loss_of_consciousness_sound;

		sound_effect_input humming_sound;
		real32 speed_contribution_to_pitch = 0.3f;

		impulse_mults drop_impulse_on_knockout = { 2000.0f, 1.5f };

		particle_effect_input health_decrease_particles;

		real32 sprint_drains_cp_per_second = 4.f;
		real32 dash_drains_cp = 15.f;
		real32 base_detached_head_speed = 2000.f;

		real32 exertion_cooldown_for_cp_regen_ms = 1000.f;

		particle_effect_input detached_head_particles;

		detached_body_parts_flavours detached_flavours;
		real32 max_inertia_when_rotation_possible = 500.f;
		real32 cp_regen_mult_when_moving = 1.f;

		real32 flash_effect_delay_ms = 200.f;

		real32 flash_audio_easing_secs = 1.5f;
		real32 flash_visual_easing_secs = 1.5f;

		real32 flash_visual_damage_mult = 0.5f;
		real32 soften_flash_until_look_mult = 0.4f;

		real32 head_hitbox_radius = 8.0f;

		particle_effect_input corpse_catch_fire_particles;
		sound_effect_input corpse_catch_fire_sound;

		real32 damage_required_for_corpse_explosion = 60.f;
		real32 corpse_burning_seconds = 0.5f;

		invariants::explosive corpse_explosion;

		corpse_remnant_flavour_vector corpse_remnant_defs;
		// END GEN INTROSPECTOR
	};
}