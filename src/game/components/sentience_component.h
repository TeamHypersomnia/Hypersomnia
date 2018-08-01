#pragma once
#include <optional>

#include "augs/graphics/rgba.h"

#include "augs/misc/timing/stepped_timing.h"
#include "augs/misc/enum/enum_map.h"
#include "augs/misc/value_meter.h"

#include "augs/templates/type_list.h"
#include "augs/templates/constexpr_arithmetic.h"
#include "augs/templates/identity_templates.h"

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

using learned_spells_array_type = std::array<
	bool,
	aligned_num_of_bytes_v<num_types_in_list_v<spell_instance_tuple>, 4>
>;

namespace components {
	struct sentience {
		// GEN INTROSPECTOR struct components::sentience
		augs::stepped_timestamp time_of_last_received_damage;
		augs::stepped_timestamp time_of_last_exertion;

		augs::stepped_cooldown cast_cooldown_for_all_spells;

		meter_instance_tuple meters;

		learned_spells_array_type learned_spells = {};

		spell_instance_tuple spells;
		perk_instance_tuple perks;

		spell_id currently_casted_spell;
		
		transformr transform_when_spell_casted;
		augs::stepped_timestamp time_of_last_spell_cast;
		augs::stepped_timestamp time_of_last_exhausted_cast;

		augs::stepped_timestamp time_of_last_shake;
		augs::stepped_timestamp when_knocked_out;
		sentience_shake shake;

		bool use_button_flag = false;
		pad_bytes<3> pad;

		use_button_state use_button = use_button_state::IDLE;
		// END GEN INTROSPECTOR

		bool is_learned(const spell_id id) const {
			ensure(id.is_set());
			return learned_spells[id.get_index()] == true;
		}

		bool is_spell_being_cast() const {
			return currently_casted_spell.is_set();
		}

		rgba calc_health_color(const float time_pulse_multiplier) const;
		std::optional<rgba> find_low_health_border(const unsigned timestamp_ms) const;

		bool is_conscious() const;

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
		real32 comfort_zone = 500.f;
		real32 minimum_danger_amount_to_evade = 20.f;
		real32 danger_amount_from_hostile_attitude = 100.f;

		real32 max_defuse_radius = 100.f;

		real32 aimpunch_impact_mult = 0.001f;
		impulse_mults knockout_impulse = { 1000.f, 80.f };

		sound_effect_input health_decrease_sound;
		sound_effect_input consciousness_decrease_sound;

		sound_effect_input death_sound;
		sound_effect_input loss_of_consciousness_sound;

		sound_effect_input humming_sound;
		real32 speed_contribution_to_pitch = 0.3f;

		impulse_mults drop_impulse_on_knockout = { 2000.0f, 1.5f };

		particle_effect_input health_decrease_particles;
		// END GEN INTROSPECTOR
	};
}