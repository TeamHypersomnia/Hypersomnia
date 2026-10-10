#pragma once

struct ai_behavior_process_ctx;

/*
	Defuse behavior.
	Bot is tasked with defusing the planted bomb.
	
	is_defusing: the bot's interaction hitbox overlaps the bomb,
	so it stands still with bare hands and requests the defuse.

	is_approaching_directly: the navmesh path to the bomb has ended
	but the bomb is still out of reach (e.g. it was planted
	on unwalkable cells), so the bot walks straight at it.
*/

struct ai_behavior_defuse {
	// GEN INTROSPECTOR struct ai_behavior_defuse
	bool is_defusing = false;
	bool is_approaching_directly = false;
	// END GEN INTROSPECTOR

	bool operator==(const ai_behavior_defuse&) const = default;

	/*
		Process defuse behavior for this frame.
		Starts defusing once the bomb is within reach.
	*/
	void process(ai_behavior_process_ctx& ctx);
};
