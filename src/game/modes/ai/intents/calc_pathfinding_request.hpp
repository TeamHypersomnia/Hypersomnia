#pragma once
#include "game/cosmos/cosmos.h"
#include "game/cosmos/entity_handle.h"
#include "game/modes/ai/arena_mode_ai_structs.h"
#include "game/modes/ai/behaviors/ai_behavior_variant.hpp"
#include "game/modes/ai/behaviors/ai_target_tracking.hpp"
#include "game/modes/ai/tasks/ai_waypoint_helpers.hpp"

/*
	Stateless calculation of the current pathfinding request.
	
	This function determines WHERE the bot wants to pathfind based on:
	- Current behavior type (COMBAT, PATROLLING, etc.)
	- Persistent state (combat_target, etc.)
	- Team state (bomb retrieval, defuse missions)
	- Game state (bomb planted, etc.)
	
	The actual pathfinding is only reinitialized when the request changes.
	This centralizes all pathfinding target decisions in one place.
	Uses std::visit on the behavior variant.
	
	Note: Push waypoints are now handled inside patrol behavior.
*/

inline std::optional<ai_pathfinding_request> calc_current_pathfinding_request(
	const cosmos& cosm,
	const ai_behavior_variant& behavior,
	const ai_target_tracking& combat_target,
	const arena_mode_ai_team_state& team_state,
	const mode_player_id& bot_player_id,
	const faction_type bot_faction,
	const vec2 character_pos,
	const bool bomb_planted,
	const entity_id bomb_entity,
	const real32 global_time_secs
) {
	(void)team_state;
	(void)bot_player_id;
	(void)bot_faction;

	return std::visit([&](const auto& b) -> std::optional<ai_pathfinding_request> {
		using T = std::decay_t<decltype(b)>;

		if constexpr (std::is_same_v<T, ai_behavior_combat>) {
			/*
				COMBAT - pathfind to last known target position.
			*/
			if (combat_target.active(global_time_secs)) {
				return ai_pathfinding_request::to_position(combat_target.last_known_pos);
			}

			return std::nullopt;
		}
		else if constexpr (std::is_same_v<T, ai_behavior_retrieve_bomb>) {
			/*
				Bomb retrieval - pathfind to bomb.
			*/
			if (bomb_entity.is_set()) {
				const auto bomb_handle = cosm[bomb_entity];

				if (bomb_handle.alive()) {
					const auto bomb_pos = bomb_handle.get_logic_transform().pos;
					return ai_pathfinding_request::to_bomb(bomb_pos);
				}
			}

			return std::nullopt;
		}
		else if constexpr (std::is_same_v<T, ai_behavior_defuse>) {
			/*
				Defuse - pathfind to bomb if not close.
			*/
			if (b.is_defusing) {
				return std::nullopt;
			}

			if (bomb_planted && bomb_entity.is_set()) {
				const auto bomb_handle = cosm[bomb_entity];

				if (bomb_handle.alive()) {
					const auto bomb_pos = bomb_handle.get_logic_transform().pos;

					if (::has_reached_waypoint(character_pos, bomb_pos, 100.0f)) {
						return std::nullopt;
					}

					return ai_pathfinding_request::to_bomb(bomb_pos);
				}
			}

			return std::nullopt;
		}
		else if constexpr (std::is_same_v<T, ai_behavior_patrol>) {
			if (b.is_camping()) {
				return std::nullopt;
			}

			const auto wp_handle = cosm[b.calc_assigned_waypoint().waypoint_id];

			if (wp_handle.alive()) {
				const auto wp_transform = wp_handle.get_logic_transform();
				return ai_pathfinding_request::to_transform(wp_transform, true);
			}

			return std::nullopt;
		}
		else if constexpr (std::is_same_v<T, ai_behavior_plant>) {
			/*
				PLANTING state - no pathfinding during plant.
			*/
			return std::nullopt;
		}
		else {
			/*
				IDLE or other - no pathfinding.
			*/
			return std::nullopt;
		}
	}, behavior);
}
