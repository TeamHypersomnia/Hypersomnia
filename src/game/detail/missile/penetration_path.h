#pragma once
#include <vector>
#include <optional>
#include <algorithm>
#include <limits>

#include "augs/math/vec2.h"
#include "game/cosmos/cosmos.h"
#include "game/cosmos/entity_handle.h"
#include "game/cosmos/entity_id.h"
#include "game/inferred_caches/physics_world_cache.h"
#include "game/enums/filters.h"
#include "game/detail/decals/penetration_fatigue.h"
#include "game/detail/physics/calc_penetrability.hpp"
#include "3rdparty/Box2D/Dynamics/b2Fixture.h"
#include "3rdparty/Box2D/Dynamics/b2Body.h"

/*
	Fixtures closer than this along the bullet's path are considered touching:
	map tiles are rarely placed with sub-pixel precision.
*/
inline constexpr real32 PENETRATION_SEAM_TOLERANCE_PX = 0.5f;

/* How far ahead penetration paths are ever traced. */
inline constexpr real32 PENETRATION_PATH_MAX_RANGE_PX = 10000.f;

/*
	Whether two fixtures belong to one wall as far as the bullet's marks are concerned:
	touching statics form one wall, while a dynamic body is a wall only of itself.
*/
inline bool same_wall(const b2Fixture& a, const b2Fixture& b) {
	if (a.GetUserData() == b.GetUserData()) {
		return true;
	}

	return
		a.GetBody()->GetType() == b2_staticBody
		&& b.GetBody()->GetType() == b2_staticBody
	;
}

struct penetration_obstacle {
	const b2Fixture* fixture = nullptr;
	entity_id owner;
	real32 penetrability = 1.f;
	real32 entry_dist = 0.f;
	real32 exit_dist = 0.f;
};

using penetration_obstacles = std::vector<penetration_obstacle>;

/*
	Gathers every penetrable obstacle along the path, in the order the bullet meets them,
	with its entry and exit distances. An obstacle the path starts inside of enters at 0,
	one it never leaves exits at max_range.

	Only reads the physics world - safe in the view, including parallel render jobs.
*/
inline void gather_penetration_obstacles(
	const cosmos& cosm,
	const vec2 from,
	const vec2 dir,
	const real32 max_range,
	const entity_id ignore_entity,
	penetration_obstacles& out
) {
	out.clear();

	const auto& physics = cosm.get_solvable_inferred().physics;
	const auto si = cosm.get_si();

	const auto filter = filters[predefined_filter_type::PENETRATING_PROGRESS_QUERY];
	const auto to = from + dir * max_range;

	const auto from_meters = si.get_meters(from);
	const auto to_meters = si.get_meters(to);

	auto find_or_add = [&](const b2Fixture* const f) -> penetration_obstacle& {
		for (auto& o : out) {
			if (o.fixture == f) {
				return o;
			}
		}

		out.push_back({ f, entity_id(), 1.f, 0.f, max_range });
		return out.back();
	};

	for (const auto& result : physics.ray_cast_all_intersections(from_meters, to_meters, filter, ignore_entity)) {
		find_or_add(result.what_fixture).entry_dist = (si.get_pixels(result.intersection) - from).dot(dir);
	}

	for (const auto& result : physics.ray_cast_all_intersections(to_meters, from_meters, filter, ignore_entity)) {
		find_or_add(result.what_fixture).exit_dist = (si.get_pixels(result.intersection) - from).dot(dir);
	}

	for (auto& o : out) {
		if (const auto owner = cosm[o.fixture->GetUserData()]) {
			o.owner = owner.get_id();
			o.penetrability = ::calc_penetrability(owner);
		}
	}

	/* Dead entities are skipped, like the logic-side simulation does. */
	out.erase(
		std::remove_if(out.begin(), out.end(), [](const penetration_obstacle& o) { return !o.owner.is_set(); }),
		out.end()
	);

	std::sort(
		out.begin(),
		out.end(),
		[](const penetration_obstacle& a, const penetration_obstacle& b) {
			if (a.entry_dist != b.entry_dist) {
				return a.entry_dist < b.entry_dist;
			}

			/* Ties are broken by identity, never by the order of the spatial query. */
			if (a.owner.raw.indirection_index != b.owner.raw.indirection_index) {
				return a.owner.raw.indirection_index < b.owner.raw.indirection_index;
			}

			return a.fixture->index_in_component < b.fixture->index_in_component;
		}
	);
}

/*
	Walks a bullet with the given power through the obstacles,
	with the same cost math as missile_system::advance_penetrations.

	on_obstacle(obstacle, end_dist) is called for every obstacle the bullet enters:
	end_dist is its exit, or the point where the bullet dies inside.

	Returns the distance at which the bullet stops,
	or std::nullopt if it makes it through all of them.
*/
template <class F>
std::optional<real32> walk_penetration_obstacles(
	const cosmos& cosm,
	const penetration_obstacles& obstacles,
	const vec2 from,
	const vec2 dir,
	real32 power,
	const real32 base_penetration_distance,
	real32 fatigue_gift_used,
	const unsigned only_born_before_step,
	F&& on_obstacle
) {
	auto at_dist = [&](const real32 d) {
		return from + dir * d;
	};

	auto cursor = 0.f;

	for (const auto& o : obstacles) {
		if (o.exit_dist <= cursor) {
			continue;
		}

		if (o.penetrability <= 0.f) {
			on_obstacle(o, o.entry_dist);
			return o.entry_dist;
		}

		const auto max_gift = ::calc_remaining_fatigue_gift(base_penetration_distance, fatigue_gift_used);

		const auto cost = ::calc_penetration_cost_px(
			cosm,
			o.owner,
			at_dist(o.entry_dist),
			at_dist(o.exit_dist),
			o.penetrability,
			max_gift,
			only_born_before_step
		);

		if (power > cost.cost) {
			power -= cost.cost;
			fatigue_gift_used += cost.gifted;

			on_obstacle(o, o.exit_dist);
			cursor = std::max(cursor, o.exit_dist);
		}
		else {
			/*
				An exact walk here: a ratio would smear the decal tunnel's discount
				uniformly over the whole obstacle, grossly undershooting on thick walls.
			*/
			const auto reach = ::calc_penetration_reach_px(
				cosm,
				o.owner,
				at_dist(o.entry_dist),
				dir,
				power,
				max_gift,
				o.penetrability,
				only_born_before_step
			);

			const auto end_dist = o.entry_dist + std::min(reach.reach, o.exit_dist - o.entry_dist);

			on_obstacle(o, end_dist);
			return end_dist;
		}
	}

	return std::nullopt;
}

/*
	How far the wall the path starts in continues along it:
	through every touching fixture that forms one wall with the first one.
*/
inline real32 calc_wall_run_end(const penetration_obstacles& obstacles) {
	const penetration_obstacle* first = nullptr;
	auto run_end = 0.f;

	for (const auto& o : obstacles) {
		/* Whatever the path merely leaves at its very start is behind it. */
		if (o.exit_dist <= PENETRATION_SEAM_TOLERANCE_PX) {
			continue;
		}

		if (first == nullptr) {
			first = &o;
			run_end = o.exit_dist;
			continue;
		}

		const bool touching = o.entry_dist <= run_end + PENETRATION_SEAM_TOLERANCE_PX;

		if (!touching || !::same_wall(*first->fixture, *o.fixture)) {
			break;
		}

		run_end = std::max(run_end, o.exit_dist);
	}

	return run_end;
}

inline penetration_obstacles& thread_local_penetration_obstacles() {
	thread_local penetration_obstacles obstacles;
	obstacles.clear();
	return obstacles;
}
