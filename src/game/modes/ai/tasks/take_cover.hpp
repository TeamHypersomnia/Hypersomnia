#pragma once
#include "game/components/movement_component.h"
#include "game/detail/inventory/weapon_reloading.hpp"
#include "game/detail/path_navigation/navigate_path.hpp"
#include "game/detail/pathfinding/pathfinding.h"
#include "game/modes/ai/ai_character_context.h"
#include "game/modes/ai/behaviors/ai_behavior_variant.hpp"
#include "game/modes/ai/intents/calc_navigation_request.hpp"
#include "game/modes/ai/tasks/danger_avoidance.hpp"

/*
	Take-cover layer — seeks cover when reloading in active combat.

	Priority: danger avoidance > take cover > bot avoidance.

	Called from PHASE 8 between update_bot_avoidance and
	update_danger_avoidance.  should_run_avoidance_update is shared with
	the other avoidance functions and throttles the BFS cover search.

	State field in arena_mode_ai_state:
	  take_cover_navigation_request — current cover target; nullopt = idle.

	Behaviour:
	  - Activates when ai_behavior_combat is active AND the bot is reloading.
	  - Cover target is computed via find_closest_cover using the last-known
	    enemy position as the primary danger and the bot's own position as the
	    secondary danger (forces the bot to go around a corner).
	  - Target is recalculated (on throttled ticks) only when target_acquired
	    is true (enemy is in penetrable line of sight).
	  - When cover is reached (path_completed), request is cleared and the
	    bot stands still until the next throttled recalculation.
	  - Terminates immediately when reloading ends: clears the request and
	    resets the current pathfinding state.
*/

inline bool update_take_cover(
	const ai_character_context& ctx,
	components::movement& movement,
	const cosmos_navmesh& navmesh,
	const navigate_path_result& move_result,
	const bool should_run_avoidance_update,
	const bool is_freeze_time,
	const bool target_acquired
) {
	if (is_freeze_time) {
		return false;
	}

	auto& ai_state = ctx.ai_state;
	const auto& cosm = ctx.cosm;
	const auto& physics = ctx.physics;
	const auto character_pos = ctx.character_pos;
	const auto si = cosm.get_si();

	const bool in_combat = ::get_behavior_if<ai_behavior_combat>(ai_state.last_behavior) != nullptr;
	const bool currently_reloading = ::is_currently_reloading(ctx.character_handle);
	const bool chambering = ::must_chamber_weapon(ctx.character_handle) && !currently_reloading;
	const bool taking_cover = in_combat && ::must_take_cover(ctx.character_handle);

	if (!taking_cover) {
		if (ai_state.take_cover_navigation_request.has_value()) {
			ai_state.take_cover_navigation_request = std::nullopt;
			ai_state.clear_navigation();
		}

		ai_state.take_cover_reached_once = false;
		return false;
	}

	/*
		Clear the request when the cover destination is reached so the bot
		stands still without triggering a pathfinding reinit loop.
		Also record that we have arrived at least once so recalculation is
		permitted on subsequent throttled ticks.
	*/
	if (move_result.path_completed
		&& ai_state.current_navigation_request == ai_state.take_cover_navigation_request
		&& ai_state.take_cover_navigation_request.has_value()
	) {
		ai_state.take_cover_navigation_request = std::nullopt;
		ai_state.take_cover_reached_once = true;
	}

	/*
		Compute or recompute the cover target on throttled ticks.
		Initial calculation: request is null (not yet found or just arrived).
		Recalculation: only once we have arrived at cover at least once and
		the enemy is now in penetrable line of sight (target_acquired).
		This prevents mid-route destination changes that would stop the bot
		from making forward progress.
	*/
	if (should_run_avoidance_update) {
		const bool need_calc = !ai_state.take_cover_navigation_request.has_value() && target_acquired;
		const bool need_recalc =
			ai_state.take_cover_navigation_request.has_value()
			&& ai_state.take_cover_reached_once
			&& target_acquired
		;

		if (need_calc || need_recalc) {
			const auto global_time_secs = static_cast<real32>(cosm.get_total_seconds_passed());

			if (ai_state.combat_target.within_engagement_window(cosm, global_time_secs)) {
				const auto enemy_pos = ai_state.combat_target.last_known_pos;

				const auto filter = predefined_queries::pathfinding();
				const auto enemy_handle = cosm[ai_state.combat_target.id];
				const bool enemy_visible = ::los_to_any_vertices_of(enemy_handle, character_pos, physics, si, filter);

				const auto secondary_danger = 
					(chambering && enemy_visible) ?
					std::optional<vec2>() :
					std::optional<vec2>(character_pos)
				;

				const auto cover_pos = ::find_closest_cover(
					navmesh,
					character_pos,
					enemy_pos,
					secondary_danger,
					physics,
					si,
					4000.0f
				);

				if (cover_pos.has_value()) {
					auto req = ai_navigation_request::to_position(*cover_pos);
					req.resolved_cell = ::resolve_cell_for_position(navmesh, *cover_pos);
					ai_state.take_cover_navigation_request = req;
					ai_state.take_cover_reached_once = false;
				}
			}
		}
	}

	/*
		Override movement for the whole duration of the reload.
		Navigate to cover when a path is active; stand still otherwise
		(either at cover or waiting for the next throttled recalculation).
	*/
	if (ai_state.take_cover_navigation_request.has_value()
		&& ai_state.current_navigation_request == ai_state.take_cover_navigation_request
		&& move_result.movement_direction.has_value()
	) {
		movement.flags.sprinting = move_result.can_sprint;
		movement.flags.set_from_closest_direction(*move_result.movement_direction);
	}
	else {
		movement.reset_movement_flags();
	}

	return true;
}
