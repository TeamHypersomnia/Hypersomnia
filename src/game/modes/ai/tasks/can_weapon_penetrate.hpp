#pragma once
#include "augs/math/vec2.h"
#include "game/cosmos/entity_handle.h"
#include "game/cosmos/cosmos.h"
#include "game/components/gun_component.h"
#include "game/components/fixtures_component.h"
#include "game/components/rigid_body_component.h"
#include "game/inferred_caches/physics_world_cache.h"
#include "game/enums/filters.h"
#include "game/detail/physics/physics_queries.h"
#include "game/detail/missile/penetration_path.h"
#include "game/modes/ai/tasks/line_of_sight.hpp"

/*
	Default threshold for penetration checks.
	A bullet must retain at least this fraction of its penetration distance
	to be considered capable of reaching the target.
*/
constexpr real32 AI_PENETRATION_THRESHOLD = 0.2f;

/*
	Walks a bullet of the bot's weapon to the target position through whatever lies between -
	with the same gathering and cost math as the weapon laser, see walk_penetration_obstacles.
	Obstacles bullets don't penetrate stop it, like it would stop the bullet.
	True if it arrives with at least threshold of its penetration distance left.
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
		The first wielded gun - the primary hand.
	*/
	const auto wielded_guns = character.get_wielded_guns();

	if (wielded_guns.empty()) {
		return false;
	}

	const auto gun_handle = cosm[wielded_guns[0]];

	if (!gun_handle.alive()) {
		return false;
	}

	const auto* const gun_invariant = gun_handle.template find<invariants::gun>();

	if (gun_invariant == nullptr) {
		return false;
	}

	const auto basic_penetration_distance = gun_invariant->basic_penetration_distance;

	const auto offset = target_pos - character_pos;
	const auto distance = offset.length();

	if (!(distance > 0.f)) {
		return true;
	}

	const auto dir = offset / distance;

	auto& obstacles = ::thread_local_penetration_obstacles();

	::gather_penetration_obstacles(
		cosm,
		character_pos,
		dir,
		distance,
		character.get_id(),
		obstacles,
		predefined_queries::bullet_penetration_check()
	);

	/*
		The step is "now" because this estimates a bullet fired right now.
	*/
	const auto walk = ::walk_penetration_obstacles(
		cosm,
		obstacles,
		character_pos,
		dir,
		basic_penetration_distance,
		basic_penetration_distance,
		0.0f,
		cosm.get_timestamp().step,
		[](auto&&...) {}
	);

	if (walk.stopped_at.has_value()) {
		return false;
	}

	if (basic_penetration_distance <= 0.0f) {
		/* Nothing on the way - and avoids dividing by 0. */
		return true;
	}

	return walk.power_left / basic_penetration_distance >= threshold;
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
