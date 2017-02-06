#pragma once
#include "game/transcendental/entity_id.h"
#include "augs/misc/recoil_player.h"
#include "augs/graphics/pixel.h"
#include "augs/misc/stepped_timing.h"

#include "augs/misc/enum_associative_array.h"

#include "game/enums/sentience_meter_type.h"

#include "game/detail/spells/haste_spell.h"

#include "game/detail/perks/haste_perk.h"

namespace components {
	struct sentience {
		struct meter {
			struct damage_result {
				float effective = 0.f;
				float ratio_effective_to_maximum = 0.f;
				bool dropped_to_zero = false;
			};

			int enabled = false;

			float value = 100.f;
			float maximum = 100.f;

			template <class Archive>
			void serialize(Archive& ar) {
				ar(
					CEREAL_NVP(enabled),

					CEREAL_NVP(value),
					CEREAL_NVP(maximum)
				);
			}

			damage_result calculate_damage_result(float amount) const;

			float ratio() const;
		};

		augs::stepped_timestamp time_of_last_received_damage;
		augs::stepped_timestamp time_of_last_exertion;

		meter health;
		meter personal_electricity;
		meter consciousness;

		haste_perk haste;

		float comfort_zone = 500.f;
		float minimum_danger_amount_to_evade = 5.f;
		float danger_amount_from_hostile_attitude = 100.f;

		recoil_player aimpunch;

		template <class Archive>
		void serialize(Archive& ar) {
			ar(
				CEREAL_NVP(health),
				CEREAL_NVP(consciousness),
				CEREAL_NVP(personal_electricity),

				CEREAL_NVP(comfort_zone),
				CEREAL_NVP(minimum_danger_amount_to_evade),
				CEREAL_NVP(danger_amount_from_hostile_attitude),

				CEREAL_NVP(aimpunch)
			);
		}

		sentience();

		meter& get(const sentience_meter_type);
		const meter& get(const sentience_meter_type) const;

		rgba calculate_health_color(float time_pulse_multiplier) const;
	};
}