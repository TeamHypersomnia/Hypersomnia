#pragma once

/*
	Defuse behavior.
	Bot is tasked with defusing the planted bomb.
	
	State tracks whether we've started the defuse action.
*/

struct ai_behavior_defuse {
	// GEN INTROSPECTOR struct ai_behavior_defuse
	bool is_defusing = false;
	// END GEN INTROSPECTOR

	bool operator==(const ai_behavior_defuse&) const = default;
};
