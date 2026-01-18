#pragma once
#include "game/cosmos/cosmos.h"
#include "game/cosmos/entity_handle.h"
#include "game/modes/ai/arena_mode_ai_structs.h"
#include "game/modes/ai/behaviors/ai_behavior_variant.hpp"
#include "game/modes/ai/behaviors/ai_target_tracking.hpp"
#include "game/modes/ai/tasks/ai_waypoint_helpers.hpp"
#include "augs/misc/randomization.h"

/*
	Round-level state used by eval_behavior_tree.
	Calculated once per frame before evaluating the behavior tree.
*/

struct ai_round_state {
	bool bomb_planted = false;
	entity_id bomb_entity;
	real32 global_time_secs = 0.0f;
};

/*
	Evaluate the behavior tree to determine the desired behavior.
	
	This function uses if/else logic to determine which behavior the bot
	should be in based on:
	- Persistent state (combat_target, patrol_letter, etc.)
	- Team common state (defuse missions, waypoint assignments)
	- Round state (bomb planted, etc.)
	
	IMPORTANTLY: This function does NOT depend on the current last_behavior.
	It only reads persistent state and game state to decide the behavior.
	
	Returns a variant containing the desired behavior with its initial
	parameters set in case we need to transition to it.
	
	Note: Push waypoints are now set in ai_behavior_patrol::push_waypoint
	instead of returning a separate ai_behavior_push.
*/

inline ai_behavior_variant eval_behavior_tree(
	const cosmos& cosm,
	const arena_mode_ai_state& ai_state,
	arena_mode_ai_team_state& team_state,
	const mode_player_id& bot_player_id,
	const faction_type bot_faction,
	const vec2 character_pos,
	const ai_round_state& round_state,
	randomization& rng
) {
	const bool is_metropolis = (bot_faction == faction_type::METROPOLIS);
	const bool is_resistance = (bot_faction == faction_type::RESISTANCE);
	const auto global_time_secs = round_state.global_time_secs;

	(void)character_pos;

	/*
		Priority 1: COMBAT if we have an active combat target.
	*/
	if (ai_state.combat_target.active(global_time_secs)) {
		return ai_behavior_combat{};
	}

	/*
		Priority 2: DEFUSE mission (Metropolis, bomb planted).
	*/
	if (round_state.bomb_planted && is_metropolis) {
		if (team_state.bot_with_defuse_mission == bot_player_id) {
			return ai_behavior_defuse{};
		}

		/*
			Assign defuse mission if not yet assigned.
		*/
		if (!team_state.bot_with_defuse_mission.is_set()) {
			team_state.bot_with_defuse_mission = bot_player_id;
			return ai_behavior_defuse{};
		}
	}

	/*
		Priority 3: RETRIEVE_BOMB mission (Resistance, bomb on ground or held by enemy).
	*/
	if (!round_state.bomb_planted && is_resistance && round_state.bomb_entity.is_set()) {
		const auto bomb_handle = cosm[round_state.bomb_entity];

		if (bomb_handle.alive()) {
			const auto bomb_owner = bomb_handle.get_owning_transfer_capability();
			const bool bomb_on_ground = !bomb_owner.alive();
			const bool bomb_held_by_enemy = bomb_owner.alive() && 
				bomb_owner.get_official_faction() == faction_type::METROPOLIS;

			if (bomb_on_ground || bomb_held_by_enemy) {
				if (team_state.bot_with_bomb_retrieval_mission == bot_player_id) {
					return ai_behavior_retrieve_bomb{};
				}

				/*
					Assign bomb retrieval mission if not yet assigned.
				*/
				if (!team_state.bot_with_bomb_retrieval_mission.is_set()) {
					team_state.bot_with_bomb_retrieval_mission = bot_player_id;
					return ai_behavior_retrieve_bomb{};
				}
			}
		}
	}

	/*
		Clear bomb retrieval mission if bomb is secured.
	*/
	if (is_resistance && team_state.bot_with_bomb_retrieval_mission == bot_player_id) {
		if (!round_state.bomb_entity.is_set()) {
			team_state.bot_with_bomb_retrieval_mission = mode_player_id::dead();
		}
		else {
			const auto bomb_handle = cosm[round_state.bomb_entity];

			if (bomb_handle.alive()) {
				const auto bomb_owner = bomb_handle.get_owning_transfer_capability();
				const bool bomb_secured = bomb_owner.alive() && 
					bomb_owner.get_official_faction() == faction_type::RESISTANCE;

				if (bomb_secured) {
					team_state.bot_with_bomb_retrieval_mission = mode_player_id::dead();
				}
			}
		}
	}

	/*
		Priority 4: PATROL behavior (with optional push_waypoint).
		
		If tried_push_already == false, we'll set the push_waypoint in the patrol behavior.
		The patrol::process() will handle clearing it once reached.
	*/
	{
		ai_behavior_patrol patrol_behavior;
		patrol_behavior.going_to_first_waypoint = true;

		/*
			If not pushed yet, try to assign a push waypoint.
		*/
		if (!ai_state.tried_push_already) {
			/* Try this only once */
			ai_state.tried_push_already = true;

			entity_id push_wp;

			if (is_resistance) {
				/*
					Resistance: try to get a push waypoint.
				*/
				push_wp = ::find_random_unassigned_push_waypoint(team_state, rng);
			}
			else if (is_metropolis) {
				/*
					Metropolis: 20% chance to choose push waypoint.
				*/
				const bool choose_push = rng.randval(0, 99) < 20;

				if (choose_push) {
					push_wp = ::find_random_unassigned_push_waypoint(team_state, rng);
				}
			}

			if (push_wp.is_set()) {
				patrol_behavior.push_waypoint = push_wp;
			}
		}

		/*
			Find an unassigned patrol waypoint in the assigned area.
			Only if we don't have a push waypoint.
		*/
		if (!patrol_behavior.push_waypoint.is_set()) {
			const auto new_wp = ::find_random_unassigned_patrol_waypoint(
				cosm,
				team_state,
				ai_state.patrol_letter,
				bot_player_id,
				entity_id::dead(),
				rng
			);

			if (new_wp.is_set()) {
				patrol_behavior.current_waypoint = new_wp;
			}
		}

		return patrol_behavior;
	}
}

/*
	Handle behavior state transitions.
	
	Called when the behavior type changes (new behavior different from last).
	Performs cleanup/setup operations needed when switching behaviors.
*/

inline void behavior_state_transition(
	const ai_behavior_variant& last_behavior,
	const ai_behavior_variant& new_behavior,
	arena_mode_ai_state& ai_state
) {
	/*
		If behavior type is the same, no transition needed.
	*/
	if (last_behavior.index() == new_behavior.index()) {
		return;
	}

	/*
		If transitioning FROM patrol with push_waypoint, mark push as done.
	*/
	if (const auto* patrol = ::get_behavior_if<ai_behavior_patrol>(last_behavior)) {
		if (patrol->push_waypoint.is_set()) {
			ai_state.tried_push_already = true;
		}
	}

	/*
		NOTE: We do NOT reset pathfinding here.
		Pathfinding resets itself when the calc_pathfinding_request changes.
		
		NOTE: We do NOT assign/unassign waypoints here.
		Waypoint assignments are handled statelessly via calc_assigned_waypoint.
	*/
}
