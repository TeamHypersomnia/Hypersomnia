#pragma once
#include <vector>
#include <optional>
#include <algorithm>
#include "augs/math/vec2.h"
#include "game/cosmos/cosmos.h"
#include "game/cosmos/entity_handle.h"
#include "game/components/fixtures_component.h"
#include "game/components/gun_component.h"
#include "game/components/rigid_body_component.h"
#include "game/inferred_caches/physics_world_cache.h"
#include "game/enums/filters.h"
#include "game/detail/missile/penetration_path.h"
#include "3rdparty/Box2D/Dynamics/b2Fixture.h"

/*
	How far the bullets of the given item penetrate - zero for anything but guns.
*/

template <class E>
real32 get_basic_penetration_distance(const E& item) {
	if (const auto* const gun_def = item.template find<invariants::gun>()) {
		return gun_def->basic_penetration_distance;
	}

	return 0.0f;
}

/*
	Where a laser from the muzzle, along the barrel, reaches the crosshair's projection on it -
	nothing when the crosshair is behind the muzzle.
*/

inline std::optional<vec2> calc_laser_end_at_crosshair(const vec2 barrel_center, const vec2 muzzle, const vec2 crosshair_pos) {
	const auto proj = crosshair_pos.get_projection_multiplier(barrel_center, muzzle);

	if (proj > 1.f) {
		return barrel_center + (muzzle - barrel_center) * proj;
	}

	return std::nullopt;
}

struct laser_path_segment {
	vec2 from;
	vec2 to;

	/*
		True when this part of the path travels inside an obstacle
		that the bullet can penetrate - drawn as a dashed line.
	*/
	bool penetrating = false;
};

/*
	Calculates the full path of a weapon laser, respecting bullet penetration.

	The path always begins with a solid segment up to the first hit of the
	bullet filter (which includes characters). If that hit is penetrable
	geometry (WALL or GLASS_OBSTACLE) and the weapon has penetration power,
	the path continues: segments inside obstacles are flagged as penetrating,
	segments in open space between obstacles are solid again. The path ends
	exactly where the simulated bullet runs out of penetration - mirroring
	the cost math of missile_system::advance_penetrations, but without
	mutating any b2Fixture scratch state, so it is safe to call from
	view code (including parallel render jobs).
	max_range is how far the path is traced at most - only as far as it can be seen.
*/

inline void calc_laser_path(
	const cosmos& cosm,
	const vec2 line_from,
	const vec2 line_to,
	const b2Filter& bullet_filter,
	const real32 basic_penetration_distance,
	const entity_id ignore_entity,
	const real32 max_range,
	std::vector<laser_path_segment>& out
) {
	const auto& physics = cosm.get_solvable_inferred().physics;
	const auto si = cosm.get_si();

	const auto laser_dir = (line_to - line_from).normalize();
	const auto far_point = line_from + laser_dir * max_range;

	const auto first_hit = physics.ray_cast_px(
		si,
		line_from,
		far_point,
		bullet_filter,
		ignore_entity
	);

	if (!first_hit.hit) {
		out.push_back({ line_from, far_point, false });
		return;
	}

	const bool first_hit_penetrable = ::is_penetrable_wall(first_hit.what_fixture->GetFilterData());

	if (!first_hit_penetrable || basic_penetration_distance <= 0.0f) {
		out.push_back({ line_from, first_hit.intersection, false });
		return;
	}

	auto& obstacles = ::thread_local_penetration_obstacles();

	::gather_penetration_obstacles(
		cosm,
		line_from,
		laser_dir,
		max_range,
		ignore_entity,
		obstacles
	);

	auto at_dist = [&](const real32 d) {
		return line_from + laser_dir * d;
	};

	auto cursor = 0.0f;

	/*
		The step is "now" because this previews a bullet fired right now.
	*/
	const auto walk = ::walk_penetration_obstacles(
		cosm,
		obstacles,
		line_from,
		laser_dir,
		basic_penetration_distance,
		basic_penetration_distance,
		0.0f,
		cosm.get_timestamp().step,
		[&](const penetration_obstacle& o, const real32 end_dist) {
			if (o.entry_dist > cursor) {
				out.push_back({ at_dist(cursor), at_dist(o.entry_dist), false });
				cursor = o.entry_dist;
			}

			const auto segment_end = std::max(end_dist, cursor);

			if (segment_end > cursor) {
				out.push_back({ at_dist(cursor), at_dist(segment_end), true });
				cursor = segment_end;
			}
		}
	);

	if (!walk.stopped_at.has_value() && cursor < max_range) {
		out.push_back({ at_dist(cursor), at_dist(max_range), false });
	}
}
