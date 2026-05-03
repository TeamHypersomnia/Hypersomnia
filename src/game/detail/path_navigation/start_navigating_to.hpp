#pragma once
#include "game/detail/pathfinding/pathfinding.h"
#include "game/modes/ai/arena_mode_ai_structs.h"
#include "game/detail/path_navigation/path_helpers.hpp"

/*
	Start navigating to a target position.
	Updates the ai_state's optional navigation field.
	Returns the new navigation state if navigation was initiated.
	
	Note: Does not initiate navigation if bot is standing on a portal cell
	(to allow portal teleportation to complete first).
*/

inline std::optional<ai_path_navigation_state> start_navigating_to(
	const vec2 bot_pos,
	const transformr target_transform,
	const cosmos_navmesh& navmesh,
	const physics_path_hints* physics_hints = nullptr,
	pathfinding_context* ctx = nullptr
) {
	/*
		Don't initiate new navigation while standing on a portal cell.
		Existing navigation sessions can continue, but new ones should wait
		until the bot has teleported through the portal.
	*/
	if (::is_on_portal_cell(bot_pos, navmesh)) {
		AI_LOG("start_navigating_to: portal");
		return std::nullopt;
	}

	const auto target_pos = target_transform.pos;
	const auto target_island_opt = ::find_island_for_position(navmesh, target_pos);

	if (!target_island_opt.has_value()) {
		AI_LOG("start_navigating_to: no island");
		return std::nullopt;
	}

	const auto target_island = *target_island_opt;
	const auto& island = navmesh.islands[target_island];
	const auto target_cell = ::world_to_cell(island, target_pos);
	const auto new_target_cell_id = cell_on_navmesh(target_island, target_cell);

	/*
		Calculate path to next portal or directly to destination.
		Using find_path_across_islands_many which only returns path to the next portal.
	*/
	auto path = ::find_path_across_islands_many(navmesh, bot_pos, target_pos, physics_hints, ctx);

	if (!path.has_value()) {
		AI_LOG("start_navigating_to: no path");
		return std::nullopt;
	}

	AI_LOG("start_navigating_to: ok");

	/*
		If this path leads to a portal (the segment ends at a portal entry cell),
		override the navigation target to be the portal's exact stand-on point
		with rotation vec2(1, 0). The portal direction itself is irrelevant for
		teleportation, but giving target_transform a defined direction lets the
		easing logic in get_navigation_movement_direction smoothly orient the
		bot as it stops on the portal cell. The bot will reorient on whatever
		segment is started after teleportation.

		exact_destination is intentionally NOT forced on here: the cell-stop
		behavior for portals is handled inside advance_path_if_cell_reached
		(it never advances past the portal node), so the bot stays on the
		portal cell regardless of the caller's exact_destination preference.
	*/
	auto effective_target_transform = target_transform;

	if (path->destination_portal_world_pos.has_value()) {
		effective_target_transform = transformr(
			*path->destination_portal_world_pos,
			vec2(1.0f, 0.0f).degrees()
		);
	}

	return ai_path_navigation_state {
		path_navigation_progress{ std::move(*path), 0 },
		std::nullopt,
		effective_target_transform,
		new_target_cell_id
	};
}
