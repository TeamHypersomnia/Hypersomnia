#pragma once
#include "entity_system/entity.h"
#include "misc/recoil_player.h"

namespace components {
	struct sentience {
		bool enable_health = true;
		bool enable_consciousness = false;

		float health = 100.f;
		float maximum_health = 100.f;
		float consciousness = 100.f;
		float maximum_consciousness = 100.f;

		float health_ratio() const {
			return health / maximum_health;
		}

		float consciousness_ratio() const {
			return consciousness / maximum_consciousness;
		}

		recoil_player aimpunch;
	};
}