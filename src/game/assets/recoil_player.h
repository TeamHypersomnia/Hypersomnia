#pragma once
#include "augs/math/vec2.h"
#include "augs/misc/constant_size_vector.h"
#include "game/container_sizes.h"
#include "game/assets/recoil_player_id.h"

class all_assets;

struct recoil_player {
	// GEN INTROSPECTOR struct recoil_player
	augs::constant_size_vector<float, RECOIL_OFFSET_COUNT> offsets;
	float fallback_random_magnitude = 90;
	// END GEN INTROSPECTOR

	recoil_player get_logical(const all_assets& manager) const {
		return *this;
	}
};

struct recoil_player_instance {
	// GEN INTROSPECTOR struct recoil_player_instance
	assets::recoil_player_id id = assets::recoil_player_id::INVALID;

	float heat_per_shot = 1;
	float cooldown_speed = 0.01f; // heat/ms

	float current_heat = 0;
	// END GEN INTROSPECTOR

	float shoot_and_get_impulse(const recoil_player& meta);

	void cooldown(float amount_ms);
};
