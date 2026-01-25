#pragma once
#include "augs/math/vec2.h"
#include "game/cosmos/entity_handle.h"
#include "game/components/gun_component.h"
#include "game/detail/gun/gun_math.h"

/*
	Estimate the aiming target position by predicting enemy movement.
	
	Takes aggressor and enemy handles, and returns a predicted enemy position
	based on:
	- Enemy's current velocity
	- Muzzle velocity of aggressor's primary hand weapon (if any)
	- Bullet travel time from muzzle to enemy
	
	If prediction is not applicable (no gun, etc.), returns enemy's current position.
*/

template <typename AggressorHandle, typename EnemyHandle>
inline vec2 estimate_aiming_target(
	const AggressorHandle& aggressor,
	const EnemyHandle& enemy
) {
	const auto& cosm = aggressor.get_cosmos();
	const auto enemy_pos = enemy.get_logic_transform().pos;

	/*
		Get enemy velocity using get_effective_velocity().
	*/
	const auto enemy_velocity = enemy.get_effective_velocity();

	/*
		If enemy isn't moving significantly, just return their current position.
	*/
	if (enemy_velocity.is_epsilon(0.1f)) {
		return enemy_pos;
	}

	/*
		Get aggressor's wielded guns.
	*/
	const auto wielded_guns = aggressor.get_wielded_guns();

	if (wielded_guns.empty()) {
		return enemy_pos;
	}

	/*
		Use the first wielded gun (primary hand).
	*/
	const auto gun_id = wielded_guns[0];
	const auto gun_handle = cosm[gun_id];

	if (!gun_handle.alive()) {
		return enemy_pos;
	}

	/*
		Get the gun's muzzle velocity (average of min and max).
	*/
	const auto* gun_invariant = gun_handle.template find<invariants::gun>();

	if (gun_invariant == nullptr) {
		return enemy_pos;
	}

	const auto muzzle_velocity_avg = (gun_invariant->muzzle_velocity.first + gun_invariant->muzzle_velocity.second) / 2.0f;

	if (muzzle_velocity_avg <= 0.0f) {
		return enemy_pos;
	}

	/*
		Calculate muzzle position using calc_muzzle_transform.
	*/
	const auto gun_transform = gun_handle.get_logic_transform();
	const auto muzzle_transform = ::calc_muzzle_transform(gun_handle, gun_transform);
	const auto muzzle_pos = muzzle_transform.pos;

	/*
		Calculate distance from muzzle to enemy.
	*/
	const auto distance_to_enemy = (enemy_pos - muzzle_pos).length();

	/*
		Estimate bullet travel time.
	*/
	const auto travel_time_secs = distance_to_enemy / muzzle_velocity_avg;

	/*
		Predict enemy position after travel time.
	*/
	const auto predicted_pos = enemy_pos + enemy_velocity * travel_time_secs;

	return predicted_pos;
}
