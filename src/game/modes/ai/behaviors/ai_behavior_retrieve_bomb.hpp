#pragma once

struct ai_behavior_process_ctx;

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
		Pickup is forced via item_pickup when pathfinding completes.
	*/

	bool operator==(const ai_behavior_retrieve_bomb&) const = default;
	void process(ai_behavior_process_ctx& ctx);
};
