#pragma once

/*
	Retrieve bomb behavior.
	Bot is tasked with picking up the dropped bomb.
	
	This behavior causes the pathfinding request to target the bomb
	and handles the pickup once reached.
*/

struct ai_behavior_retrieve_bomb {
	/*
		Bomb retrieval doesn't need internal state.
		The pathfinding target is determined from the bomb entity position.
		Pickup happens automatically via normal item interaction.
	*/

	bool operator==(const ai_behavior_retrieve_bomb&) const = default;
	void process(ai_behavior_process_ctx&) {}
};
