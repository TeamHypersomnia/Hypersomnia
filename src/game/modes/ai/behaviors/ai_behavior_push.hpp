#pragma once
#include <optional>
#include "augs/misc/randomization.h"
#include "game/cosmos/entity_id.h"

struct ai_behavior_patrol;
struct arena_mode_ai_state;
struct arena_mode_ai_team_state;
struct mode_player_id;

/*
	Push behavior.
	Bot is moving towards a push waypoint before switching to patrol.
	
	Push waypoints are used at round start for aggressive forward positions.
	Once reached, the bot transitions to patrol.
*/

struct ai_behavior_push {
	// GEN INTROSPECTOR struct ai_behavior_push
	entity_id target_waypoint;
	// END GEN INTROSPECTOR

	bool operator==(const ai_behavior_push&) const = default;

	/*
		Process push behavior for this frame.
		Handles path completion by transitioning to patrol.
		
		Returns the new patrol behavior if transition should occur, nullopt otherwise.
	*/
	template <typename T>
	std::optional<ai_behavior_patrol> process(
		T& cosm,
		arena_mode_ai_state& ai_state,
		arena_mode_ai_team_state& team_state,
		const mode_player_id& bot_player_id,
		randomization& rng,
		const bool pathfinding_just_completed
	);
};
