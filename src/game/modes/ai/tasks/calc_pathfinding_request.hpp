#pragma once
#include "game/cosmos/cosmos.h"
#include "game/cosmos/entity_handle.h"
#include "game/modes/ai/arena_mode_ai_structs.h"
#include "game/modes/ai/tasks/ai_waypoint_helpers.hpp"

/*
	Stateless calculation of the current pathfinding request.
	
	This function determines WHERE the bot wants to pathfind based on:
	- Current behavioral state (COMBAT, PATROLLING, PUSHING, etc.)
	- Team state (bomb retrieval, defuse missions)
	- Game state (bomb planted, etc.)
	
	The actual pathfinding is only reinitialized when the request changes.
	This centralizes all pathfinding target decisions in one place.
*/

inline ai_pathfinding_request calc_current_pathfinding_request(
	const cosmos& cosm,
	const arena_mode_ai_state& ai_state,
	const arena_mode_ai_team_state& team_state,
	const mode_player_id& bot_player_id,
	const faction_type bot_faction,
	const vec2 character_pos,
	const bool bomb_planted,
	const entity_id bomb_entity
) {
	const bool is_metropolis = (bot_faction == faction_type::METROPOLIS);
	const bool is_resistance = (bot_faction == faction_type::RESISTANCE);

	/*
		Priority 1: COMBAT - pathfind to last known target position.
	*/
	if (ai_state.is_in_combat()) {
		return ai_pathfinding_request::to_position(ai_state.last_known_target_pos);
	}

	/*
		Priority 2: Bomb retrieval (Resistance, bomb not planted).
	*/
	if (!bomb_planted && is_resistance && bomb_entity.is_set()) {
		const auto bomb_handle = cosm[bomb_entity];

		if (bomb_handle.alive()) {
			const auto bomb_owner = bomb_handle.get_owning_transfer_capability();
			const bool bomb_on_ground = !bomb_owner.alive();
			const bool bomb_held_by_enemy = bomb_owner.alive() && 
				bomb_owner.get_official_faction() == faction_type::METROPOLIS;

			if ((bomb_on_ground || bomb_held_by_enemy) && 
			    team_state.bot_with_bomb_retrieval_mission == bot_player_id) {
				const auto bomb_pos = bomb_handle.get_logic_transform().pos;
				return ai_pathfinding_request::to_bomb(bomb_pos);
			}
		}
	}

	/*
		Priority 3: Defuse mission (Metropolis, bomb planted).
	*/
	if (bomb_planted && is_metropolis && 
	    team_state.bot_with_defuse_mission == bot_player_id && bomb_entity.is_set()) {
		const auto bomb_handle = cosm[bomb_entity];

		if (bomb_handle.alive()) {
			const auto bomb_pos = bomb_handle.get_logic_transform().pos;

			/*
				If close to bomb (within 100px), don't pathfind - enter defuse mode.
			*/
			if (::has_reached_waypoint(character_pos, bomb_pos, 100.0f)) {
				return ai_pathfinding_request::none();
			}

			return ai_pathfinding_request::to_bomb(bomb_pos);
		}
	}

	/*
		Priority 4: PUSHING state - pathfind to push waypoint.
	*/
	if (ai_state.current_state == bot_state_type::PUSHING && ai_state.current_waypoint.is_set()) {
		const auto wp_handle = cosm[ai_state.current_waypoint];

		if (wp_handle.alive()) {
			const auto wp_transform = wp_handle.get_logic_transform();
			return ai_pathfinding_request::to_transform(wp_transform, true);
		}
	}

	/*
		Priority 5: PATROLLING state - pathfind to patrol waypoint.
	*/
	if (ai_state.current_state == bot_state_type::PATROLLING && ai_state.current_waypoint.is_set()) {
		/*
			If camping (timer > 0), don't pathfind - stay in place and twitch.
		*/
		if (::is_camping(ai_state)) {
			return ai_pathfinding_request::none();
		}

		const auto wp_handle = cosm[ai_state.current_waypoint];

		if (wp_handle.alive()) {
			const auto wp_transform = wp_handle.get_logic_transform();
			return ai_pathfinding_request::to_transform(wp_transform, true);
		}
	}

	/*
		No pathfinding needed.
	*/
	return ai_pathfinding_request::none();
}
