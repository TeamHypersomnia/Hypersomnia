#pragma once
#include "entity_system/entity.h"
#include "misc/recoil_player.h"

namespace components {
	struct sentience {
		bool enable_health = true;
		bool enable_consciousness = false;

		float health = 100.f;
		float consciousness = 100.f;

		recoil_player aimpunch;
	};
}