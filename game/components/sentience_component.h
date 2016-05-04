#pragma once
#include "entity_system/entity.h"
#include "misc/recoil_player.h"
#include "graphics/pixel.h"

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

		augs::rgba calculate_health_color(float time_pulse_multiplier) const;

		recoil_player aimpunch;
	};
}