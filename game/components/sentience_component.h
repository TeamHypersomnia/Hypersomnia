#pragma once
#include "entity_system/entity.h"
#include "misc/recoil_player.h"
#include "graphics/pixel.h"

namespace components {
	struct sentience {
		bool enable_health = true;
		bool enable_consciousness = false;

		float health = 100.f;
		float maximum_health = 100.f;
		float consciousness = 100.f;
		float maximum_consciousness = 100.f;

		float health_ratio() const;
		float consciousness_ratio() const;
		augs::rgba calculate_health_color(float time_pulse_multiplier) const;

		recoil_player aimpunch;
	};
}