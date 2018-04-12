#pragma once
#include "augs/math/vec2.h"
#include "augs/misc/constant_size_vector.h"
#include "game/container_sizes.h"
#include "game/assets/ids/recoil_player_id.h"

struct recoil_player {
	// GEN INTROSPECTOR struct recoil_player
	augs::constant_size_vector<real32, RECOIL_OFFSET_COUNT> offsets = {};
	std::string name;
	// END GEN INTROSPECTOR
};

struct recoil_player_instance_def {
	// GEN INTROSPECTOR struct recoil_player_instance_def
	assets::recoil_player_id id = assets::recoil_player_id::INVALID;

	real32 heat_per_shot = 1;
	real32 heat_cooldown_per_ms = 0.01f;
	// END GEN INTROSPECTOR
};

struct recoil_player_instance {
	// GEN INTROSPECTOR struct recoil_player_instance
	real32 current_heat = 0;
	// END GEN INTROSPECTOR

	real32 shoot_and_get_impulse(
		const recoil_player_instance_def& def,
		const recoil_player& meta
	);

	void cooldown(
		const recoil_player_instance_def& def,
		real32 amount_ms
	);

	void cooldown(real32 amount_ms);
};
