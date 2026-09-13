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
	const auto far_point = line_from + laser_dir * 10000;

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

	/*
		Gather entry and exit points of every penetrable obstacle along the
		whole path with two all-intersection raycasts (forward and backward),
		paired per-fixture in a local vector instead of the b2Fixture
		scratch fields used by the logic-side simulation.
	*/

	struct walk_entry {
		const b2Fixture* fixture = nullptr;
		std::optional<real32> entry_dist;
		std::optional<real32> exit_dist;
	};

	std::vector<walk_entry> obstacles;

	auto find_or_add = [&](const b2Fixture* const f) -> walk_entry& {
		for (auto& o : obstacles) {
			if (o.fixture == f) {
				return o;
			}
		}

		obstacles.push_back({ f, std::nullopt, std::nullopt });
		return obstacles.back();
	};

	const auto progress_filter = filters[predefined_filter_type::PENETRATING_PROGRESS_QUERY];

	const auto p1_meters = si.get_meters(line_from);
	const auto p2_meters = si.get_meters(far_point);

	for (const auto& result : physics.ray_cast_all_intersections(p1_meters, p2_meters, progress_filter, ignore_entity)) {
		auto& o = find_or_add(result.what_fixture);
		o.entry_dist = (si.get_pixels(result.intersection) - line_from).dot(laser_dir);
	}

	for (const auto& result : physics.ray_cast_all_intersections(p2_meters, p1_meters, progress_filter, ignore_entity)) {
		auto& o = find_or_add(result.what_fixture);
		o.exit_dist = (si.get_pixels(result.intersection) - line_from).dot(laser_dir);
	}

	const auto max_range = (far_point - line_from).length();

	std::sort(
		obstacles.begin(),
		obstacles.end(),
		[](const walk_entry& a, const walk_entry& b) {
			return a.entry_dist.value_or(0.0f) < b.entry_dist.value_or(0.0f);
		}
	);

	auto penetrability_of = [&](const b2Fixture& fixture) -> std::optional<real32> {
		if (const auto handle = cosm[fixture.GetUserData()]) {
			auto penetrability = 1.0f;

			if (const auto* const fixtures_comp = handle.template find<invariants::fixtures>()) {
				penetrability = fixtures_comp->penetrability;
			}

			if (const auto body = handle.template find<components::rigid_body>()) {
				penetrability *= body.get_special().penetrability;
			}

			return penetrability;
		}

		return std::nullopt;
	};

	auto at_dist = [&](const real32 d) {
		return line_from + laser_dir * d;
	};

	auto penetration_remaining = basic_penetration_distance;
	auto cursor = 0.0f;

	for (const auto& o : obstacles) {
		const auto entry_d = o.entry_dist.value_or(0.0f);
		const auto exit_d = o.exit_dist.value_or(max_range);

		if (exit_d <= cursor) {
			continue;
		}

		if (entry_d > cursor) {
			out.push_back({ at_dist(cursor), at_dist(entry_d), false });
			cursor = entry_d;
		}

		const auto maybe_penetrability = penetrability_of(*o.fixture);

		if (!maybe_penetrability.has_value()) {
			/* Dead entity - skip, like the logic-side simulation does. */
			continue;
		}

		const auto penetrability = *maybe_penetrability;

		if (penetrability <= 0.0f) {
			return;
		}

		const auto full_penetrated_distance = (exit_d - entry_d) / penetrability;

		if (penetration_remaining > full_penetrated_distance) {
			penetration_remaining -= full_penetrated_distance;

			const auto seg_end = std::max(exit_d, cursor);
			out.push_back({ at_dist(cursor), at_dist(seg_end), true });
			cursor = seg_end;
		}
		else {
			/*
				The bullet dies inside this obstacle - same expression as
				missile_system's expire_at(considered_p1 + offset * ratio).
			*/
			const auto end_d = std::max(entry_d + penetration_remaining * penetrability, cursor);
			out.push_back({ at_dist(cursor), at_dist(end_d), true });
			return;
		}
	}

	if (cursor < max_range) {
		out.push_back({ at_dist(cursor), at_dist(max_range), false });
	}
}
