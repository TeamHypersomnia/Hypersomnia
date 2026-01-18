#pragma once
#include "game/detail/pathfinding.h"
#include "game/modes/ai/arena_mode_ai_structs.h"
#include "game/detail/pathfinding/path_helpers.hpp"

/*
	Start pathfinding to a target position.
	Updates the ai_state's optional pathfinding field.
	Returns true if pathfinding was initiated.
	
	Note: Does not initiate pathfinding if bot is standing on a portal cell
	(to allow portal teleportation to complete first).
*/

inline bool start_pathfinding_to(
	arena_mode_ai_state& ai_state,
	const vec2 bot_pos,
	const transformr target_transform,
	const cosmos_navmesh& navmesh,
	pathfinding_context* ctx
) {
	/*
		Check if we're already pathfinding to this destination.
	*/
	const auto target_pos = target_transform.pos;
	const auto target_island_opt = ::find_island_for_position(navmesh, target_pos);

	if (!target_island_opt.has_value()) {
		return false;
	}

	const auto target_island = *target_island_opt;
	const auto& island = navmesh.islands[target_island];
	const auto target_cell = ::world_to_cell(island, target_pos);
	const auto new_target_cell_id = cell_on_navmesh(target_island, target_cell);

	if (ai_state.is_pathfinding_active() &&
	    ai_state.pathfinding->target_cell_id == new_target_cell_id
	) {
		/*
			Already navigating to the same destination.
		*/
		return true;
	}

	/*
		Check if we're already at the target cell.
		If so, don't start pathfinding - we've already arrived.
		This prevents restarting pathfinding after path completed.
	*/
	if (::is_within_cell(bot_pos, island, target_cell)) {
		return false;
	}

	/*
		Calculate path to next portal or directly to destination.
		Using find_path_across_islands_many which only returns path to the next portal.
	*/
	auto path = ::find_path_across_islands_many(navmesh, bot_pos, target_pos, ctx);

	if (!path.has_value()) {
		return false;
	}

	ai_state.pathfinding = ai_pathfinding_state {
		pathfinding_progress{ std::move(*path), 0 },
		std::nullopt,
		target_transform,
		new_target_cell_id
	};

	return true;
}

/*
	Overload for std::optional<ai_pathfinding_state> - useful for test_mode.
*/

inline bool start_pathfinding_to(
	std::optional<ai_pathfinding_state>& pathfinding_opt,
	const vec2 bot_pos,
	const transformr target_transform,
	const cosmos_navmesh& navmesh,
	pathfinding_context* ctx
) {
	/*
		Don't initiate new pathfinding while standing on a portal cell.
		Existing pathfinding sessions can continue, but new ones should wait
		until the bot has teleported through the portal.
	*/
	if (!pathfinding_opt.has_value() && ::is_on_portal_cell(bot_pos, navmesh)) {
		return false;
	}

	/*
		Check if we're already pathfinding to this destination.
	*/
	const auto target_pos = target_transform.pos;
	const auto target_island_opt = ::find_island_for_position(navmesh, target_pos);

	if (!target_island_opt.has_value()) {
		return false;
	}

	const auto target_island = *target_island_opt;
	const auto& island = navmesh.islands[target_island];
	const auto target_cell = ::world_to_cell(island, target_pos);
	const auto new_target_cell_id = cell_on_navmesh(target_island, target_cell);

	if (pathfinding_opt.has_value() &&
	    pathfinding_opt->target_cell_id == new_target_cell_id) {
		/*
			Already navigating to the same destination.
		*/
		return true;
	}

	/*
		Check if we're already at the target cell.
		If so, don't start pathfinding - we've already arrived.
		This prevents restarting pathfinding after path completed.
	*/
	if (::is_within_cell(bot_pos, island, target_cell)) {
		return false;
	}

	/*
		Calculate path to next portal or directly to destination.
		Using find_path_across_islands_many which only returns path to the next portal.
	*/
	auto path = ::find_path_across_islands_many(navmesh, bot_pos, target_pos, ctx);

	if (!path.has_value()) {
		return false;
	}

	pathfinding_opt = ai_pathfinding_state{
		pathfinding_progress{ std::move(*path), 0 },
		std::nullopt,
		target_transform,
		new_target_cell_id
	};

	return true;
}
