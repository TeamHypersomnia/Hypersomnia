#pragma once
#include "game/entity_id.h"
#include "misc/recoil_player.h"
#include "augs/graphics/pixel.h"

namespace components {
	struct sentience {
		sentience();

		struct meter {
			bool enabled = false;

			float value = 100.f;
			float maximum = 100.f;

			struct damage_result {
				float effective = 0.f;
				float ratio_effective_to_maximum = 0.f;
				bool dropped_to_zero = false;
			};

			damage_result calculate_damage_result(float amount) const;

			float ratio() const;
		};

		meter health;
		meter consciousness;
		meter shield;

		float comfort_zone = 500.f;
		float minimum_danger_amount_to_evade = 5.f;
		float danger_amount_from_hostile_attitude = 100.f;

		augs::rgba calculate_health_color(float time_pulse_multiplier) const;

		recoil_player aimpunch;
	};
}