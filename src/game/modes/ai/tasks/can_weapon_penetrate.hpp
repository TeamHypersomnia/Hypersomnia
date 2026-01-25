#pragma once
#include <vector>
#include "augs/math/vec2.h"
#include "game/cosmos/entity_handle.h"
#include "game/cosmos/cosmos.h"
#include "game/components/gun_component.h"
#include "game/components/fixtures_component.h"
#include "game/components/rigid_body_component.h"
#include "game/inferred_caches/physics_world_cache.h"
#include "game/enums/filters.h"

/*
	Simulates bullet penetration to determine if the bot's weapon can penetrate
	through obstacles to reach the target position.
	
	Uses the same penetration logic as missile_system.cpp:
	- Raycast forward and backward to find all intersected fixtures
	- Calculate penetration cost based on fixture penetrability
	- Return true if remaining penetration / basic_penetration_distance >= threshold
	
	This allows bots to shoot at targets through walls when their weapon
	has sufficient penetration power.
*/

template <typename CharacterHandle>
inline bool can_weapon_penetrate(
	const CharacterHandle& character,
	const vec2 target_pos,
	const real32 threshold = 0.4f
) {
	const auto& cosm = character.get_cosmos();
	const auto character_pos = character.get_logic_transform().pos;

	/*
		Get wielded guns - need at least one to check penetration.
	*/
	const auto wielded_guns = character.get_wielded_guns();

	if (wielded_guns.empty()) {
		return false;
	}

	/*
		Use the first wielded gun (primary hand).
	*/
	const auto gun_id = wielded_guns[0];
	const auto gun_handle = cosm[gun_id];

	if (!gun_handle.alive()) {
		return false;
	}

	/*
		Get the gun's basic_penetration_distance.
	*/
	const auto* gun_invariant = gun_handle.template find<invariants::gun>();

	if (gun_invariant == nullptr) {
		return false;
	}

	const auto basic_penetration_distance = gun_invariant->basic_penetration_distance;

	if (basic_penetration_distance <= 0.0f) {
		return false;
	}

	/*
		Perform raycasts to simulate penetration.
		Use FLYING_BULLET filter like the missile system does.
	*/
	const auto& physics = cosm.get_solvable_inferred().physics;
	const auto si = cosm.get_si();
	const auto filter = filters[predefined_filter_type::FLYING_BULLET];

	const auto p1 = character_pos;
	const auto p2 = target_pos;
	const auto p1_meters = si.get_meters(p1);
	const auto p2_meters = si.get_meters(p2);

	/*
		Raycast forward (p1 -> p2) to get entry points.
		Raycast backward (p2 -> p1) to get exit points.
		We need to match entry/exit points for the same fixture to calculate thickness.
	*/
	const auto forward_results = physics.ray_cast_all_intersections(p1_meters, p2_meters, filter, character);
	const auto backward_results = physics.ray_cast_all_intersections(p2_meters, p1_meters, filter, character);

	/*
		Build a map from fixture -> (entry, exit) points.
		Entry points come from forward raycast, exit points from backward raycast.
	*/
	struct fixture_segment {
		vec2 entry = vec2::zero;
		vec2 exit = vec2::zero;
		bool has_entry = false;
		bool has_exit = false;
		b2Fixture* fixture = nullptr;
	};

	std::vector<fixture_segment> segments;

	auto find_or_create_segment = [&](b2Fixture* f) -> fixture_segment& {
		for (auto& seg : segments) {
			if (seg.fixture == f) {
				return seg;
			}
		}
		segments.push_back({});
		segments.back().fixture = f;
		return segments.back();
	};

	for (const auto& result : forward_results) {
		auto& seg = find_or_create_segment(result.what_fixture);
		seg.entry = si.get_pixels(result.intersection);
		seg.has_entry = true;
	}

	for (const auto& result : backward_results) {
		auto& seg = find_or_create_segment(result.what_fixture);
		seg.exit = si.get_pixels(result.intersection);
		seg.has_exit = true;
	}

	/*
		Calculate total penetration cost.
		For each segment, cost = thickness / penetrability.
		
		Following missile_system.cpp logic:
		- If penetrability <= 0, the obstacle is impenetrable
		- Otherwise, cost is proportional to thickness and inversely proportional to penetrability
	*/
	real32 penetration_remaining = basic_penetration_distance;

	for (const auto& seg : segments) {
		if (seg.fixture == nullptr) {
			continue;
		}

		/*
			Get penetrability from fixtures and rigid body components.
		*/
		real32 penetrability = 1.0f;

		if (const auto handle = cosm[seg.fixture->GetUserData()]) {
			if (const auto fixtures_comp = handle.template find<invariants::fixtures>()) {
				penetrability = fixtures_comp->penetrability;
			}

			if (const auto body = handle.template find<components::rigid_body>()) {
				penetrability *= body.get_special().penetrability;
			}
		}
		else {
			continue;
		}

		/*
			If impenetrable, we cannot reach the target.
		*/
		if (penetrability <= 0.0f) {
			return false;
		}

		/*
			Calculate thickness and penetration cost.
			
			If we have both entry and exit, use them.
			If we only have entry (bullet starts inside), use p1 as entry.
			If we only have exit (bullet ends inside), use p2 as exit.
		*/
		vec2 considered_entry = seg.has_entry ? seg.entry : p1;
		vec2 considered_exit = seg.has_exit ? seg.exit : p2;

		const auto thickness = (considered_exit - considered_entry).length();
		const auto penetration_cost = thickness / penetrability;

		penetration_remaining -= penetration_cost;

		if (penetration_remaining <= 0.0f) {
			/*
				Ran out of penetration - check if we've passed the threshold.
			*/
			return false;
		}
	}

	/*
		Check if remaining penetration ratio meets the threshold.
	*/
	const auto remaining_ratio = penetration_remaining / basic_penetration_distance;
	return remaining_ratio >= threshold;
}
