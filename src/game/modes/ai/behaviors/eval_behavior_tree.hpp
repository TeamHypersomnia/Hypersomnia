#pragma once
#include "game/cosmos/cosmos.h"
#include "game/cosmos/entity_handle.h"
#include "game/detail/sentience/sentience_getters.h"
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
	arena_mode_ai_state& ai_state,
	arena_mode_ai_team_state& team_state,
	const arena_mode_ai_arena_meta& arena_meta,
	const mode_player_id& bot_player_id,
	const entity_id controlled_character_id,
	const faction_type bot_faction,
	const vec2 character_pos,
	const ai_round_state& round_state,
	randomization& rng
) {
	const bool is_metropolis = (bot_faction == faction_type::METROPOLIS);
	const bool is_resistance = (bot_faction == faction_type::RESISTANCE);
	const auto global_time_secs = round_state.global_time_secs;

	(void)character_pos;
	(void)bot_player_id;

	/*
		Priority 1: COMBAT if we have an active combat target.
	*/
	if (ai_state.combat_target.active(cosm, global_time_secs)) {
		AI_LOG("eval_behavior_tree: COMBAT (faction=%x, bomb_planted=%x)", static_cast<int>(bot_faction), round_state.bomb_planted);
		return ai_behavior_combat{};
	}

	/*
		Priority 2: DEFUSE mission (Metropolis, bomb planted).
	*/
	if (round_state.bomb_planted && is_metropolis) {
		if (team_state.bot_with_defuse_mission == controlled_character_id) {
			return ai_behavior_defuse{};
		}

		/*
			Assign defuse mission if no bot is assigned or previous assignee is dead/unconscious.
		*/
		if (!sentient_and_conscious(cosm[team_state.bot_with_defuse_mission])) {
			team_state.bot_with_defuse_mission = controlled_character_id;
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
				if (team_state.bot_with_bomb_retrieval_mission == controlled_character_id) {
					return ai_behavior_retrieve_bomb{};
				}

				/*
					Assign bomb retrieval mission if no bot is assigned or previous assignee is dead/unconscious.
				*/
				if (!sentient_and_conscious(cosm[team_state.bot_with_bomb_retrieval_mission])) {
					team_state.bot_with_bomb_retrieval_mission = controlled_character_id;
					return ai_behavior_retrieve_bomb{};
				}
			}
		}
	}

	/*
		Clear bomb retrieval mission if bomb is secured.
	*/
	if (is_resistance && team_state.bot_with_bomb_retrieval_mission == controlled_character_id) {
		if (!round_state.bomb_entity.is_set()) {
			team_state.bot_with_bomb_retrieval_mission = entity_id::dead();
		}
		else {
			const auto bomb_handle = cosm[round_state.bomb_entity];

			if (bomb_handle.alive()) {
				const auto bomb_owner = bomb_handle.get_owning_transfer_capability();
				const bool bomb_secured = bomb_owner.alive() && 
					bomb_owner.get_official_faction() == faction_type::RESISTANCE;

				if (bomb_secured) {
					team_state.bot_with_bomb_retrieval_mission = entity_id::dead();
				}
			}
		}
	}

	/*
		Priority 4: PLANT behavior (Resistance, bot has the bomb, push phase completed).
		
		If the bot has the bomb in inventory and has already completed the push waypoint
		(or skipped it), then planting takes priority before combat.
	*/
	if (!round_state.bomb_planted && is_resistance && ai_state.push_phase == push_phase_type::COMPLETED) {
		if (round_state.bomb_entity.is_set()) {
			const auto bomb_handle = cosm[round_state.bomb_entity];

			if (bomb_handle.alive()) {
				const auto bomb_owner = bomb_handle.get_owning_transfer_capability();
				const bool is_bomb_carrier = bomb_owner == cosm[controlled_character_id];

				if (is_bomb_carrier) {
					return ai_behavior_plant{};
				}
			}
		}
	}

	/*
		Priority 5: PATROL behavior (with optional push_waypoint).
		
		If push_phase == NOT_DECIDED, assign a push waypoint and transition to IN_PROGRESS.
		Once the push waypoint is reached, patrol_process sets push_phase = COMPLETED.
	*/
	{
		auto patrol_behavior = ai_behavior_patrol();

		/*
			If push hasn't been decided yet, assign a push waypoint (once per round).
		*/
		if (ai_state.push_phase == push_phase_type::NOT_DECIDED) {
			/*
				Determine whether this bot is the bomb carrier.
			*/
			bool is_bomb_carrier = false;

			if (is_resistance && round_state.bomb_entity.is_set()) {
				const auto bomb_handle = cosm[round_state.bomb_entity];

				if (bomb_handle.alive()) {
					is_bomb_carrier = bomb_handle.get_owning_transfer_capability() == cosm[controlled_character_id];
				}
			}

			entity_id push_wp;

			if (is_resistance) {
				/*
					Resistance non-carriers always push.
					Bomb carrier pushes with 80% probability.
				*/
				const bool should_push = !is_bomb_carrier || rng.randval(0, 99) < 80;

				if (should_push) {
					push_wp = ::find_random_unassigned_push_waypoint(team_state, rng);
				}
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
				ai_state.push_phase = push_phase_type::IN_PROGRESS;

				/*
					When the bomb carrier is assigned a push waypoint, align
					chosen_bombsite to the nearest bomb area so the team plants
					at the site that makes sense for the chosen approach.
				*/
				if (is_bomb_carrier) {
					const auto push_wp_pos = cosm[push_wp].get_logic_transform().pos;
					const auto closest = ::find_closest_bombsite_letter(cosm, arena_meta, push_wp_pos);

					if (closest != marker_letter_type::COUNT) {
						team_state.chosen_bombsite = closest;
					}
				}
			}
			else {
				/*
					No push waypoint available (or carrier skipped push) —
					push phase is immediately done so the bomb carrier can plant.
				*/
				ai_state.push_phase = push_phase_type::COMPLETED;
			}
		}

		if (is_resistance && round_state.bomb_planted) {
			AI_LOG(
				"eval_behavior_tree: PATROL (faction=%x, bomb_planted=%x, patrol_letter=%x, push_phase=%x, push_wp_set=%x)",
				static_cast<int>(bot_faction), round_state.bomb_planted,
				static_cast<int>(ai_state.patrol_letter), static_cast<int>(ai_state.push_phase),
				patrol_behavior.push_waypoint.is_set()
			);
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
		If transitioning FROM patrol while push is still in progress
		(e.g. interrupted by combat or a higher-priority mission),
		mark push as completed so the bot can plant when it returns.
	*/
	if (const auto* patrol = ::get_behavior_if<ai_behavior_patrol>(last_behavior)) {
		if (patrol->push_waypoint.is_set() && ai_state.push_phase == push_phase_type::IN_PROGRESS) {
			ai_state.push_phase = push_phase_type::COMPLETED;
		}
	}

	/*
		NOTE: We do NOT reset pathfinding here.
		Pathfinding resets itself when the calc_pathfinding_request changes.
		
		NOTE: We do NOT assign/unassign waypoints here.
		Waypoint assignments are handled statelessly via calc_assigned_waypoint.
	*/
}
