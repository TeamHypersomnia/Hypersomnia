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
#include "game/detail/decals/penetration_fatigue.h"
#include "game/detail/physics/calc_penetrability.hpp"
#include "game/modes/ai/tasks/line_of_sight.hpp"

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
	const bool zero_penetration =  basic_penetration_distance <= 0.0f;

	/*
		Perform raycasts to simulate penetration.
		Use FLYING_BULLET filter like the missile system does.
		
		Interpret p1 as the "previous tip" (character pos) and p2 as "current tip" (target pos),
		following the same logic as missile_system::advance_penetrations.
	*/
	const auto& physics = cosm.get_solvable_inferred().physics;
	const auto si = cosm.get_si();
	const auto filter = predefined_queries::bullet_penetration_check();

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
	real32 fatigue_gift_used = 0.0f;
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

		const auto surface = cosm[fixture.GetUserData()];

		if (surface.dead()) {
			continue;
		}

		const auto surface_owner = surface.get_id();
		const auto penetrability = ::calc_penetrability(surface);

		const auto considered_p1 = fixture.penetrated_forward ? vec2(fixture.forward_point) : p1;
		const auto considered_p2 = fixture.penetrated_backward ? vec2(fixture.backward_point) : p2;

		if (penetrability <= 0.0f) {
			can_penetrate = false;
			break;
		}
		else {
			/*
				Discounted by material fatigue - same math as the missile system.
				The step is "now" because this estimates a bullet fired right now.
			*/
			const auto max_gift = ::calc_remaining_fatigue_gift(basic_penetration_distance, fatigue_gift_used);

			const auto cost = ::calc_penetration_cost_px(
				cosm,
				surface_owner,
				considered_p1,
				considered_p2,
				penetrability,
				max_gift,
				cosm.get_timestamp().step
			);

			fatigue_gift_used += cost.gifted;

			if (penetration_remaining > cost.cost) {
				penetration_remaining -= cost.cost;
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

	if (zero_penetration) {
		/* Avoid div by 0 */
		return true;
	}

	/*
		Check if remaining penetration ratio meets the threshold.
	*/
	const auto remaining_ratio = penetration_remaining / basic_penetration_distance;
	return remaining_ratio >= threshold;
}

/*
	"Can my current attack reach target_handle?"

	With a wielded gun, defers to can_weapon_penetrate against target_pos
	(covers both clear paths and wall penetration).

	Without a gun (melee or bare hands), uses los_to_any_vertices_of so a
	partially-visible target still counts as visible — mirroring how
	find_closest_enemy decides visual contact. Whether melee actually connects
	(swing range) is decided downstream by calc_hand_flags.
*/

template <typename CharacterHandle, typename TargetHandle>
inline bool can_attack_position(
	const CharacterHandle& character,
	const TargetHandle& target_handle,
	const vec2 target_pos,
	const real32 threshold = AI_PENETRATION_THRESHOLD
) {
	if (!character.get_wielded_guns().empty()) {
		return ::can_weapon_penetrate(character, target_pos, threshold);
	}

	const auto& cosm = character.get_cosmos();
	const auto& physics = cosm.get_solvable_inferred().physics;
	const auto si = cosm.get_si();
	const auto character_pos = character.get_logic_transform().pos;
	const auto filter = predefined_queries::bullet_penetration_check();

	return ::los_to_any_vertices_of(target_handle, character_pos, physics, si, filter);
}
