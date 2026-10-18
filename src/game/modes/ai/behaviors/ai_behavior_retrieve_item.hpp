#pragma once

struct ai_behavior_process_ctx;

/*
	Retrieve item behavior.
	Outside combat and buying, a bot with room for one more firearm goes for the nearest one lying around
	(arena_mode_ai_state::item_to_retrieve, found in arena_mode_ai.cpp) and picks it up once reached.
*/

struct ai_behavior_retrieve_item {
	bool operator==(const ai_behavior_retrieve_item&) const = default;
	void process(ai_behavior_process_ctx& ctx);
};
