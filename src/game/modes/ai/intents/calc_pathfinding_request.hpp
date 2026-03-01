#pragma once
#include "game/cosmos/cosmos.h"
#include "game/cosmos/entity_handle.h"
#include "game/cosmos/for_each_entity.h"
#include "game/modes/ai/arena_mode_ai_structs.h"
#include "game/modes/ai/behaviors/ai_behavior_variant.hpp"
#include "game/modes/ai/behaviors/ai_target_tracking.hpp"
#include "game/modes/ai/tasks/ai_waypoint_helpers.hpp"
#include "game/detail/pathfinding.h"
#include "game/detail/pathfinding_bomb.hpp"
#include "game/components/marker_component.h"

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
	
	Now also resolves the cell_on_navmesh for efficient comparison.
	For bomb targets, uses find_bomb_pathfinding_target to get the resolved cell.
*/

/*
	Helper to resolve a world position to a cell_on_navmesh.
*/
inline cell_on_navmesh resolve_cell_for_position(
	const cosmos_navmesh& navmesh,
	const vec2 world_pos
) {
	const auto island_opt = ::find_island_for_position(navmesh, world_pos);
	if (!island_opt.has_value()) {
		return cell_on_navmesh();
	}
	
	const auto island_idx = *island_opt;
	const auto& island = navmesh.islands[island_idx];
	const auto cell = ::world_to_cell(island, world_pos);
	
	return cell_on_navmesh(island_idx, cell);
}

/*
	Helper to create a pathfinding request from a bomb_pathfinding_target.
	Now uses the resolved_cell from bomb_pathfinding_target directly
	to avoid redundant recalculation.
*/
inline std::optional<ai_pathfinding_request> create_bomb_pathfinding_request(
	const std::optional<bomb_pathfinding_target>& bomb_target
) {
	if (!bomb_target.has_value()) {
		return std::nullopt;
	}
	
	auto req = ai_pathfinding_request::to_transform(bomb_target->target_pathfinding_transform);
	req.resolved_cell = bomb_target->resolved_cell;
	return req;
}

inline std::optional<ai_pathfinding_request> calc_current_pathfinding_request(
	const cosmos& cosm,
	ai_behavior_variant& behavior,
	const ai_target_tracking& combat_target,
	const arena_mode_ai_team_state& team_state,
	const arena_mode_ai_arena_meta& arena_meta,
	const mode_player_id& bot_player_id,
	const faction_type bot_faction,
	const vec2 character_pos,
	const bool bomb_planted,
	const entity_id bomb_entity,
	const real32 global_time_secs,
	const cosmos_navmesh& navmesh,
	randomization& rng
) {
	(void)bot_player_id;
	(void)global_time_secs;

	return std::visit([&](auto& b) -> std::optional<ai_pathfinding_request> {
		using T = std::decay_t<decltype(b)>;

		if constexpr (std::is_same_v<T, ai_behavior_combat>) {
			/*
				COMBAT - pathfind to last known target position.
			*/
			auto req = ai_pathfinding_request::to_position(combat_target.last_known_pos);
			req.resolved_cell = ::resolve_cell_for_position(navmesh, combat_target.last_known_pos);
			return req;
		}
		else if constexpr (std::is_same_v<T, ai_behavior_retrieve_bomb>) {
			/*
				Bomb retrieval - pathfind to bomb.
			*/
			if (bomb_entity.is_set()) {
				const auto bomb_handle = cosm[bomb_entity];

				if (bomb_handle.alive()) {
					const auto bomb_target = ::find_bomb_pathfinding_target(bomb_handle, navmesh, character_pos);
					return ::create_bomb_pathfinding_request(bomb_target);
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
					const auto bomb_target = ::find_bomb_pathfinding_target(bomb_handle, navmesh, character_pos);
					return ::create_bomb_pathfinding_request(bomb_target);
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
				auto req = ai_pathfinding_request::to_transform(wp_transform, true);
				req.resolved_cell = ::resolve_cell_for_position(navmesh, wp_transform.pos);
				return req;
			}

			return std::nullopt;
		}
		else if constexpr (std::is_same_v<T, ai_behavior_plant>) {
			/*
				PLANTING state - no pathfinding during plant.
				Otherwise, pathfind to cached bombsite target.
			*/
			if (b.is_planting) {
				return std::nullopt;
			}

			/*
				If cached_plant_target is not set, find a random unoccupied cell
				within the chosen bombsite area using gathered bombsite mappings.
			*/
			if (!b.cached_plant_target.has_value()) {
				const auto bombsite_letter = team_state.chosen_bombsite;
				const auto* bombsite_ids = arena_meta.find_bombsite_ids(bombsite_letter);

				if (bombsite_ids != nullptr) {
					std::optional<transformr> found_target;

					for (const auto& bombsite_id : *bombsite_ids) {
						if (found_target.has_value()) {
							break;
						}

						const auto typed_handle = cosm[bombsite_id];

						if (!typed_handle.alive()) {
							continue;
						}

						/*
							Get AABB, transform, and size for the bombsite.
						*/
						if (const auto aabb = typed_handle.find_aabb()) {
							const auto rect_transform = typed_handle.get_logic_transform();
							const auto rect_size = typed_handle.get_logical_size();

							const auto random_pos = ::find_random_unoccupied_cell_within_rect(
								navmesh,
								*aabb,
								rect_transform,
								rect_size,
								rng
							);

							if (random_pos.has_value()) {
								/*
									Choose a random direction for the plant transform.
								*/
								const auto random_degrees = rng.randval(0.0f, 360.0f);
								found_target = transformr(*random_pos, random_degrees);
							}
						}
					}

					if (found_target.has_value()) {
						b.cached_plant_target = *found_target;
					}
				}
			}

			/*
				Return cached target if available.
			*/
			if (b.cached_plant_target.has_value()) {
				const auto& target = *b.cached_plant_target;
				auto req = ai_pathfinding_request::to_transform(target, true);
				req.resolved_cell = ::resolve_cell_for_position(navmesh, target.pos);
				return req;
			}

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
