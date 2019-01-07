#pragma once
#include <optional>

#include "augs/graphics/rgba.h"

#include "augs/misc/timing/stepped_timing.h"
#include "augs/misc/enum/enum_map.h"
#include "augs/misc/value_meter.h"

#include "augs/templates/type_list.h"
#include "augs/templates/constexpr_arithmetic.h"
#include "augs/templates/identity_templates.h"
#include "augs/misc/constant_size_vector.h"

#include "game/assets/ids/asset_ids.h"

#include "game/cosmos/entity_id.h"

#include "game/components/transform_component.h"

#include "game/enums/use_button_state.h"

#include "game/detail/all_sentience_meters.h"
#include "augs/math/physics_structs.h"
#include "game/detail/spells/all_spells.h"
#include "game/detail/perks/all_perks.h"
#include "game/detail/sentience_shake.h"

#include "game/detail/view_input/sound_effect_input.h"
#include "game/detail/view_input/particle_effect_input.h"
#include "game/detail/damage_origin.h"

#include "game/detail/sentience/detached_body_parts.h"
#include "game/enums/use_button_query_result.h"
#include "game/enums/weapon_action_type.h"
#include "game/detail/inventory/hand_count.h"

struct damage_owner {
	// GEN INTROSPECTOR struct damage_owner
	signi_entity_id who;
	meter_value_type amount = 0;
	// END GEN INTROSPECTOR

	bool operator<(const damage_owner& b) const {
		/* Bigger come first */
		return amount > b.amount;
	}
};

using damage_owners_vector = augs::constant_size_vector<damage_owner, 3>;

namespace components {
	struct sentience {
		// GEN INTROSPECTOR struct components::sentience
		augs::stepped_timestamp time_of_last_received_damage;
		augs::stepped_timestamp time_of_last_exertion;

		augs::stepped_cooldown cast_cooldown_for_all_spells;

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
		sentience_shake shake;

		use_button_state use_button = use_button_state::IDLE;
		use_button_query_result last_use_result = use_button_query_result::NONE_FOUND;

		damage_owners_vector damage_owners;
		damage_origin knockout_origin;

		detached_body_parts detached;

		std::array<bool, hand_count_v> hand_flags = {};
		bool block_flag = false;
		pad_bytes<1> pad;

		std::array<augs::stepped_timestamp, hand_count_v> when_hand_pressed = {};

		real32 rotation_inertia_ms = 0.1f;
		// END GEN INTROSPECTOR

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

namespace invariants {
	struct sentience {
		// GEN INTROSPECTOR struct invariants::sentience
		real32 shake_mult = 0.5f;
		real32 dash_impulse_mult = 1.f;

		real32 comfort_zone = 500.f;
		real32 minimum_danger_amount_to_evade = 20.f;
		real32 danger_amount_from_hostile_attitude = 100.f;

		real32 use_button_radius = 100.f;
		real32 use_button_angle = 90.f;

		real32 aimpunch_mult = 1.f;
		real32 const_inertia_damage_ratio = 2.f;
		real32 linear_inertia_damage_ratio = 1.f;

		impulse_mults knockout_impulse = { 1000.f, 80.f };

		sound_effect_input health_decrease_sound;
		sound_effect_input consciousness_decrease_sound;

		sound_effect_input death_sound;
		sound_effect_input loss_of_consciousness_sound;

		sound_effect_input humming_sound;
		real32 speed_contribution_to_pitch = 0.3f;

		impulse_mults drop_impulse_on_knockout = { 2000.0f, 1.5f };

		particle_effect_input health_decrease_particles;

		real32 minimum_cp_to_sprint = 0.1f;
		real32 sprint_drains_cp_per_second = 4.f;
		real32 dash_drains_cp = 15.f;
		real32 base_detached_head_speed = 2000.f;

		real32 shield_damage_absorption_mult = 0.5f;

		particle_effect_input detached_head_particles;

		detached_body_parts_flavours detached_flavours;
		real32 max_inertia_when_rotation_possible = 500.f;
		// END GEN INTROSPECTOR
	};
}