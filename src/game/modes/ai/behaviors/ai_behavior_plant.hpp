#pragma once

/*
	Plant behavior.
	Bot is tasked with planting the bomb at a bombsite.
	
	State tracks whether we've started the plant action.
*/

struct ai_behavior_plant {
	// GEN INTROSPECTOR struct ai_behavior_plant
	bool is_planting = false;
	// END GEN INTROSPECTOR

	bool operator==(const ai_behavior_plant&) const = default;
};
