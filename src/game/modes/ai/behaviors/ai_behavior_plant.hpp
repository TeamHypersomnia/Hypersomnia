#pragma once
#include <optional>
#include "augs/math/transform.h"

struct ai_behavior_process_ctx;

/*
	Plant behavior.
	Bot is tasked with planting the bomb at a bombsite.
	
	State tracks whether we've started the plant action.
	
	The cached_plant_target is used to store the randomly chosen
	bombsite cell for pathfinding. Once set, it's reused until:
	- Planting is interrupted (when_started_arming becomes unset)
	- Behavior is changed
*/

struct ai_behavior_plant {
	// GEN INTROSPECTOR struct ai_behavior_plant
	bool is_planting = false;
	std::optional<transformr> cached_plant_target;
	// END GEN INTROSPECTOR

	bool operator==(const ai_behavior_plant&) const = default;

	/*
		Process plant behavior for this frame.
		Handles:
		- Detecting plant interruption via when_started_arming
		- Starting plant when path is completed
	*/
	void process(ai_behavior_process_ctx& ctx);
};
