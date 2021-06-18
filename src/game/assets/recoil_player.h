#pragma once
#include <vector>
#include "augs/math/vec2.h"
#include "game/assets/ids/asset_ids.h"

struct recoil_player {
	// GEN INTROSPECTOR struct recoil_player
	std::vector<real32> offsets = {};
	std::string name;
	// END GEN INTROSPECTOR

	const auto& get_name() const {
		return name;
	}
};

struct recoil_player_instance_def {
	// GEN INTROSPECTOR struct recoil_player_instance_def
	assets::recoil_player_id id;

	real32 pattern_progress_per_shot = 1;
	real32 pattern_progress_damping = 10.0f;
	// END GEN INTROSPECTOR
};

struct recoil_player_instance {
	// GEN INTROSPECTOR struct recoil_player_instance
	real32 pattern_progress = 0;
	// END GEN INTROSPECTOR

	real32 shoot_and_get_impulse(
		const recoil_player_instance_def& def,
		const recoil_player& meta
	);

	void cooldown(
		const recoil_player_instance_def& def,
		real32 dt_secs
	);

	void cooldown(real32 amount_ms);
};
