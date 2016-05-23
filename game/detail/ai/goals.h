#pragma once
#include <vector>
#include "entity_system/entity_id.h"
#include "game/detail/entity_scripts.h"

namespace behaviours {
	struct immediate_evasion_goal {
		std::vector<identified_danger> dangers;
	};

	typedef std::tuple<behaviours::immediate_evasion_goal> goal_tuple;
}
