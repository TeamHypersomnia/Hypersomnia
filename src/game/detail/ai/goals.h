#pragma once
#include <tuple>
#include <vector>

#include "game/cosmos/entity_id.h"
#include "game/detail/entity_scripts.h"

namespace behaviours {
	struct immediate_evasion_goal {
		std::vector<identified_danger> dangers;
	};

	struct minimize_recoil_through_movement_goal {
		vec2 movement_direction;
	};

	using goal_tuple = std::tuple<
		behaviours::immediate_evasion_goal,
		behaviours::minimize_recoil_through_movement_goal
	>;
}
