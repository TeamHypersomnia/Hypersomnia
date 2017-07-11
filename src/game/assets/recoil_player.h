#pragma once
#include "augs/math/vec2.h"
#include "augs/misc/constant_size_vector.h"
#include "game/container_sizes.h"
#include "game/assets/recoil_player_id.h"

class assets_manager;

struct recoil_player {
	// GEN INTROSPECTOR struct recoil_player
	augs::constant_size_vector<vec2, RECOIL_OFFSET_COUNT> offsets;
	// END GEN INTROSPECTOR

	recoil_player get_logical_meta(const assets_manager& manager) const {
		return *this;
	}
};

struct recoil_player_instance {
	// GEN INTROSPECTOR struct recoil_player_instance
	assets::recoil_player_id id = assets::recoil_player_id::INVALID;
	std::size_t index = 0;
	// END GEN INTROSPECTOR

	vec2 shoot_and_get_impulse(const recoil_player& meta);
};
