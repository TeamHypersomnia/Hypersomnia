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
#include "game/detail/physics/physics_queries.h"

/*
	Default threshold for penetration checks.
	A bullet must retain at least this fraction of its penetration distance
	to be considered capable of reaching the target.
*/
constexpr real32 AI_PENETRATION_THRESHOLD = 0.2f;

/*
	Simulates bullet penetration to determine if the bot's weapon can penetrate
	through obstacles to reach the target position.
	
	Uses the same penetration logic as missile_system::advance_penetrations:
	- p1 (character pos) is treated as the "previous tip"
	- p2 (target pos) is treated as the "current tip"
	- Uses b2Fixture fields: forward_point, backward_point, penetrated_forward, 
	  penetrated_backward, penetration_processed_flag
	- Calculate penetration cost based on fixture penetrability
	- Return true if remaining penetration / basic_penetration_distance >= threshold
	
	This allows bots to shoot at targets through walls when their weapon
	has sufficient penetration power.
*/

template <typename CharacterHandle>
inline bool can_weapon_penetrate(
	const CharacterHandle& character,
	const vec2 target_pos,
	const real32 threshold = AI_PENETRATION_THRESHOLD
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
		
		Interpret p1 as the "previous tip" (character pos) and p2 as "current tip" (target pos),
		following the same logic as missile_system::advance_penetrations.
	*/
	const auto& physics = cosm.get_solvable_inferred().physics;
	const auto si = cosm.get_si();
	const auto filter = filters[predefined_filter_type::FLYING_BULLET];

	const auto p1 = character_pos;
	const auto p2 = target_pos;
	const auto p1_meters = si.get_meters(p1);
	const auto p2_meters = si.get_meters(p2);

	/*
		Collect all hit fixtures using the b2Fixture fields directly,
		just like missile_system::advance_penetrations does.
	*/
	std::vector<b2Fixture*> hits;

	/* Fill forward facing hits */
	{
		const auto results = physics.ray_cast_all_intersections(p1_meters, p2_meters, filter, character);

		for (const auto& result : results) {
			auto f = result.what_fixture;
			f->penetrated_forward = true;
			f->forward_point = b2Vec2(si.get_pixels(result.intersection));
			hits.push_back(f);
		}
	}

	/* Fill backward facing hits */
	{
		const auto results = physics.ray_cast_all_intersections(p2_meters, p1_meters, filter, character);

		for (const auto& result : results) {
			auto f = result.what_fixture;
			f->penetrated_backward = true;
			f->backward_point = b2Vec2(si.get_pixels(result.intersection));
			hits.push_back(f);
		}
	}

	/*
		Calculate penetration through all hit fixtures.
		Following missile_system.cpp logic exactly.
	*/
	real32 penetration_remaining = basic_penetration_distance;
	bool can_penetrate = true;

	for (auto& fixture_ptr : hits) {
		if (fixture_ptr == nullptr) {
			continue;
		}

		auto& fixture = *fixture_ptr;

		if (fixture.penetration_processed_flag) {
			continue;
		}

		fixture.penetration_processed_flag = true;

		float penetrability = 1.0f;

		if (const auto handle = cosm[fixture.GetUserData()]) {
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

		const auto considered_p1 = fixture.penetrated_forward ? vec2(fixture.forward_point) : p1;
		const auto considered_p2 = fixture.penetrated_backward ? vec2(fixture.backward_point) : p2;

		if (penetrability <= 0.0f) {
			can_penetrate = false;
			break;
		}
		else {
			const auto offset = considered_p2 - considered_p1;
			const auto full_penetrated_distance = offset.length() / penetrability;

			if (penetration_remaining > full_penetrated_distance) {
				penetration_remaining -= full_penetrated_distance;
			}
			else {
				can_penetrate = false;
				break;
			}
		}
	}

	/* Cleanup - reset the fixture flags we used */
	for (auto& fixture : hits) {
		if (fixture == nullptr) {
			continue;
		}

		fixture->penetration_processed_flag = false;
		fixture->penetrated_forward = false;
		fixture->penetrated_backward = false;
	}

	if (!can_penetrate) {
		return false;
	}

	/*
		Check if remaining penetration ratio meets the threshold.
	*/
	const auto remaining_ratio = penetration_remaining / basic_penetration_distance;
	return remaining_ratio >= threshold;
}
