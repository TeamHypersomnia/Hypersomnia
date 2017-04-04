#pragma once
#include "augs/graphics/pixel.h"

#include "augs/misc/recoil_player.h"
#include "augs/misc/stepped_timing.h"
#include "augs/misc/enum_associative_array.h"

#include "game/transcendental/entity_id.h"

#include "game/enums/sentience_meter_type.h"
#include "game/enums/spell_type.h"

#include "game/assets/sound_buffer_id.h"

#include "game/components/transform_component.h"

#include "game/detail/spell_logic.h"
#include "game/detail/perks/haste_perk.h"
#include "game/detail/perks/electric_shield_perk.h"
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

		sound_response health_decrease_sound_response;
		sound_response death_sound_response;

		// END GEN INTROSPECTOR

		// calls abstraction for GUI

	private:
		template <class F>
		decltype(auto) redirect_arguments_now_dt(
			const sentience_meter_type type,
			F callback,
			const augs::stepped_timestamp now,
			const augs::delta dt
		) const {
			switch (type) {
				case sentience_meter_type::HEALTH: return callback(health);
				case sentience_meter_type::PERSONAL_ELECTRICITY: return callback(personal_electricity);
				case sentience_meter_type::CONSCIOUSNESS: return callback(consciousness);
				case sentience_meter_type::HASTE: return callback(haste.timing, now, dt);
				case sentience_meter_type::ELECTRIC_SHIELD: return callback(electric_shield.timing, now, dt);
				default: ensure("unknown sentience meter type" && false); return callback(health);
			}
		}

	public:

		bool is_enabled(
			const sentience_meter_type type,
			const augs::stepped_timestamp now,
			const augs::delta dt
		) const {
			return redirect_arguments_now_dt(type, [](const auto& t, auto... args) { return t.is_enabled(args...); }, now, dt);
		}
		
		float get_ratio(
			const sentience_meter_type type,
			const augs::stepped_timestamp now,
			const augs::delta dt
		) const {
			return redirect_arguments_now_dt(type, [](const auto& t, auto... args) { return t.get_ratio(args...); }, now, dt);
		}

		meter_value_type get_value(
			const sentience_meter_type type,
			const augs::stepped_timestamp now,
			const augs::delta dt
		) const {
			switch (type) {
			case sentience_meter_type::HEALTH: 
				return health.get_value();
			case sentience_meter_type::PERSONAL_ELECTRICITY: 
				return personal_electricity.get_value();
			case sentience_meter_type::CONSCIOUSNESS: 
				return consciousness.get_value();
			case sentience_meter_type::HASTE: 
				return static_cast<meter_value_type>(haste.timing.duration.cooldown_duration_ms * haste.timing.get_ratio(now, dt));
			case sentience_meter_type::ELECTRIC_SHIELD: 
				return static_cast<meter_value_type>(electric_shield.timing.duration.cooldown_duration_ms * electric_shield.timing.get_ratio(now, dt));
			default: ensure("unknown sentience meter type" && false); return 0;
			}
		}

		meter_value_type get_maximum_value(
			const sentience_meter_type type
		) const {
			switch (type) {
			case sentience_meter_type::HEALTH:
				return health.get_maximum_value();
			case sentience_meter_type::PERSONAL_ELECTRICITY:
				return personal_electricity.get_maximum_value();
			case sentience_meter_type::CONSCIOUSNESS:
				return consciousness.get_maximum_value();
			case sentience_meter_type::HASTE:
				return static_cast<meter_value_type>(haste.timing.duration.cooldown_duration_ms);
			case sentience_meter_type::ELECTRIC_SHIELD:
				return static_cast<meter_value_type>(electric_shield.timing.duration.cooldown_duration_ms);
			default: ensure("unknown sentience meter type" && false); return 0;
			}
		}

		rgba calculate_health_color(float time_pulse_multiplier) const;
		bool is_conscious() const;
	};
}