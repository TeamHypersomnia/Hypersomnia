#pragma once
#include <vector>
#include <optional>
#include <algorithm>
#include "augs/math/vec2.h"
#include "game/cosmos/cosmos.h"
#include "game/cosmos/entity_handle.h"
#include "game/components/fixtures_component.h"
#include "game/components/rigid_body_component.h"
#include "game/inferred_caches/physics_world_cache.h"
#include "game/enums/filters.h"
#include "game/detail/missile/penetration_path.h"
#include "3rdparty/Box2D/Dynamics/b2Fixture.h"

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
*/

inline void calc_laser_path(
	const cosmos& cosm,
	const vec2 line_from,
	const vec2 line_to,
	const b2Filter& bullet_filter,
	const real32 basic_penetration_distance,
	const entity_id ignore_entity,
	std::vector<laser_path_segment>& out
) {
	const auto& physics = cosm.get_solvable_inferred().physics;
	const auto si = cosm.get_si();

	const auto laser_dir = (line_to - line_from).normalize();
	const auto far_point = line_from + laser_dir * PENETRATION_PATH_MAX_RANGE_PX;

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

	const auto first_hit_category = first_hit.what_fixture->GetFilterData().categoryBits;

	const auto penetrable_categories = uint16(
		(1 << int(filter_category::WALL)) |
		(1 << int(filter_category::GLASS_OBSTACLE))
	);

	const bool first_hit_penetrable = (first_hit_category & penetrable_categories) != 0;

	if (!first_hit_penetrable || basic_penetration_distance <= 0.0f) {
		out.push_back({ line_from, first_hit.intersection, false });
		return;
	}

	auto& obstacles = ::thread_local_penetration_obstacles();

	::gather_penetration_obstacles(
		cosm,
		line_from,
		laser_dir,
		PENETRATION_PATH_MAX_RANGE_PX,
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
	const auto stopped_at = ::walk_penetration_obstacles(
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

	if (!stopped_at.has_value() && cursor < PENETRATION_PATH_MAX_RANGE_PX) {
		out.push_back({ at_dist(cursor), at_dist(PENETRATION_PATH_MAX_RANGE_PX), false });
	}
}
