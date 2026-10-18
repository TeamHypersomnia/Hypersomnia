#pragma once
#include <optional>
#include "augs/math/vec2.h"
#include "augs/misc/randomization.h"
#include "game/cosmos/cosmos.h"
#include "game/enums/filters.h"
#include "game/inferred_caches/physics_world_cache.h"
#include "game/detail/pathfinding/pathfinding.h"
#include "game/modes/ai/arena_mode_ai_structs.h"
#include "game/modes/ai/behaviors/ai_behavior_variant.hpp"
#include "game/modes/ai/tasks/ai_waypoint_helpers.hpp"
#include "game/modes/ai/tasks/ai_field_of_view.h"

/*
	A bot that hears an enemy it neither sees nor engages turns to where the enemy would show up:
	along the bot's shortest path to the sound, the last point still in its line of sight -
	the visibility discontinuity on the enemy's route. The path goes through free space,
	so the bot never stares into a wall, and from a cramped corner it faces the corridor out.
	Only a standing bot turns - camping, or waiting with no free waypoint (or idle) - a walking one keeps facing its way.
*/

/*
	The approach is recomputed at most this often - the newest sound waits in heard_for_approach meanwhile.
	Every bot in another step of the interval (by its index), so the bots hearing the same steps never path all at once.
*/

static constexpr real32 heard_approach_interval_secs = 0.5f;

/*
	How far along the path to the sound the last point in sight is looked for - binary search over its cells,
	so at most log2 of their count in rays.
*/

static constexpr real32 heard_approach_max_path_px = 6144.0f;

inline std::optional<vec2> find_heard_approach_point(
	const cosmos& cosm,
	const cosmos_navmesh& navmesh,
	const physics_world_cache& physics,
	const vec2 bot_pos,
	const vec2 heard_pos,
	pathfinding_context* const pathfinding_ctx
) {
	const auto path = ::find_path_across_islands_many(navmesh, bot_pos, heard_pos, nullptr, pathfinding_ctx);

	if (!path.has_value() || path->nodes.empty() || path->island_index >= navmesh.islands.size()) {
		return std::nullopt;
	}

	const auto& island = navmesh.islands[path->island_index];
	const auto max_cells = std::max(1, static_cast<int>(heard_approach_max_path_px) / static_cast<int>(std::max(1u, island.cell_size)));
	const auto num = std::min(static_cast<int>(path->nodes.size()), max_cells);
	const auto si = cosm.get_si();

	auto point_of = [&](const int i) {
		return ::cell_to_world(island, path->nodes[static_cast<std::size_t>(i)].cell_xy);
	};

	auto in_sight = [&](const int i) {
		return !physics.ray_cast_px(si, bot_pos, point_of(i), predefined_queries::line_of_sight()).hit;
	};

	/*
		A path cell in sight followed by one out of sight - where the enemy shows up.
		Visibility along the path need not be monotonic, so it is any such transition the binary search lands on.
		Cell 0 is the bot's own: its center out of sight, or the turn right at it, says nothing about the direction.
	*/

	if (!in_sight(0)) {
		return std::nullopt;
	}

	auto lo = 0;
	auto hi = num;

	while (hi - lo > 1) {
		const auto mid = (lo + hi) / 2;

		if (in_sight(mid)) {
			lo = mid;
		}
		else {
			hi = mid;
		}
	}

	if (lo == 0) {
		return std::nullopt;
	}

	return point_of(lo);
}

/*
	Called every step after the behaviors are processed. Returns where to aim (an offset from the bot) if it should face an approach.
*/

inline std::optional<vec2> face_heard_approach(
	const cosmos& cosm,
	const cosmos_navmesh& navmesh,
	const physics_world_cache& physics,
	arena_mode_ai_state& ai_state,
	const vec2 bot_pos,
	const ai_field_of_view& fov,
	const real32 dt_secs,
	const std::size_t bot_index,
	const bool is_freeze_time,
	pathfinding_context* const pathfinding_ctx,
	randomization& rng
) {
	if (ai_state.approach_hold_timer > 0.0f) {
		ai_state.approach_hold_timer -= dt_secs;
	}

	auto* const patrol = ::get_behavior_if<ai_behavior_patrol>(ai_state.last_behavior);

	const bool camping = patrol != nullptr && patrol->is_camping();
	const bool waiting = !ai_state.is_navigating() && (
		(patrol != nullptr && !camping && !patrol->patrol_waypoint.is_set() && !patrol->push_waypoint.is_set())
		|| ::is_behavior<ai_behavior_idle>(ai_state.last_behavior)
	);

	if (is_freeze_time || ai_state.is_in_combat() || (!camping && !waiting)) {
		/*
			A sound heard while walking or fighting is stale by the time the bot stands again.
		*/

		ai_state.approach_hold_timer = 0.0f;
		ai_state.heard_for_approach_pending = false;
		return std::nullopt;
	}

	const auto steps_per_check = std::max(1u, static_cast<uint32_t>(heard_approach_interval_secs / dt_secs + 0.5f));
	const bool check_due = (cosm.get_total_steps_passed() + bot_index) % steps_per_check == 0;

	if (ai_state.heard_for_approach_pending && check_due) {
		ai_state.heard_for_approach_pending = false;

		if (const auto point = ::find_heard_approach_point(cosm, navmesh, physics, bot_pos, ai_state.heard_for_approach, pathfinding_ctx)) {
			const auto dir = vec2(*point - bot_pos).normalize();

			if (dir.is_nonzero()) {
				/*
					Stays for 50-100% of a camp - not a fixed time a player could wait out.
				*/

				const auto share = rng.randval(0.5f, 1.0f);

				if (camping) {
					const auto [min_secs, max_secs] = ::get_waypoint_camp_duration_range(cosm, patrol->calc_assigned_waypoint().waypoint_id);
					const auto allowed = std::max(0.0f, max_secs * 2.0f - patrol->camp_stay_secs);
					const auto extended = std::min(rng.randval(min_secs, max_secs) * share, allowed);

					/*
						Where a bot looking far already sees, it keeps looking far the same way.
					*/

					const bool already_in_view = patrol->camp_look_far && fov.within_angle(patrol->camp_look_direction, dir);

					if (!already_in_view) {
						patrol->camp_look_direction = dir;
						patrol->camp_look_far = false;
					}

					if (extended > patrol->camp_timer) {
						patrol->camp_timer = extended;
					}
				}
				else {
					ai_state.approach_look_point = *point;
					ai_state.approach_hold_timer = rng.randval(2.0f, 10.0f) * share;
				}
			}
		}
	}

	if (waiting && ai_state.approach_hold_timer > 0.0f) {
		return vec2(ai_state.approach_look_point - bot_pos).normalize() * 200.0f;
	}

	return std::nullopt;
}
