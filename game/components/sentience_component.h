#pragma once
#include "game/transcendental/entity_id.h"
#include "augs/misc/recoil_player.h"
#include "augs/graphics/pixel.h"
#include "augs/misc/stepped_timing.h"

#include "augs/misc/enum_associative_array.h"

#include "game/enums/perk_meter_type.h"
#include "game/enums/sentience_meter_type.h"
#include "game/enums/spell_type.h"

#include "game/detail/spell_logic.h"

#include "game/detail/perks/haste_perk.h"
#include "game/detail/perks/electric_shield_perk.h"
#include "game/components/transform_component.h"

#include "game/detail/sentience_meter.h"

namespace components {
	struct sentience {
		// GEN INTROSPECTOR struct components::sentience
		augs::stepped_timestamp time_of_last_received_damage;
		augs::stepped_timestamp time_of_last_exertion;

		augs::stepped_cooldown cast_cooldown_for_all_spells;

		sentience_meter health;
		sentience_meter personal_electricity;
		sentience_meter consciousness;

		haste_perk haste;
		electric_shield_perk electric_shield;

		augs::enum_associative_array<spell_type, spell_instance_data> spells;

		spell_type currently_casted_spell = spell_type::COUNT;
		components::transform transform_when_spell_casted;
		augs::stepped_timestamp time_of_last_spell_cast;
		augs::stepped_timestamp time_of_last_exhausted_cast;

		augs::stepped_timestamp time_of_last_shake;
		float shake_for_ms = 0.f;

		float comfort_zone = 500.f;
		float minimum_danger_amount_to_evade = 5.f;
		float danger_amount_from_hostile_attitude = 100.f;

		recoil_player aimpunch;
		child_entity_id health_damage_particles;
		child_entity_id character_crosshair;
		// END GEN INTROSPECTOR

		template <class F>
		decltype(auto) call_on(
			const sentience_meter_type s,
			const F callback
		) const {
			switch (s) {
			case sentience_meter_type::HEALTH: return callback(health);
			case sentience_meter_type::PERSONAL_ELECTRICITY: return callback(personal_electricity);
			case sentience_meter_type::CONSCIOUSNESS: return callback(consciousness);
			default: ensure(false); return callback(health);
			}
		}

		template <class F>
		decltype(auto) call_on(
			const perk_meter_type s,
			const F callback
		) const {
			switch (s) {
			case perk_meter_type::HASTE: return callback(haste);
			case perk_meter_type::ELECTRIC_SHIELD: return callback(electric_shield);
			default: ensure(false); return callback(haste);
			}
		}

		rgba calculate_health_color(float time_pulse_multiplier) const;
		bool is_conscious() const;
	};
}