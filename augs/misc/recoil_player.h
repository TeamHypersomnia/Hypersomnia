#pragma once
#include <vector>
#include "augs/math/vec2.h"
#include "game/transcendental/entity_handle_declaration.h"

#include "augs/misc/constant_size_vector.h"
#include "game/container_sizes.h"

class recoil_player {
	int delta_offset = 0;
public:
	// GEN INTROSPECTOR recoil_player
	augs::constant_size_vector<vec2, RECOIL_PLAYER_OFFSET_COUNT> offsets;
	unsigned current_offset = 0;
	int reversed = false;
	unsigned repeat_last_n_offsets = 5;

	float single_cooldown_duration_ms = 50.0;
	float remaining_cooldown_duration = -1.0;
	float scale = 1.0;
	// END GEN INTROSPECTOR

	vec2 shoot_and_get_offset();
	
	void shoot_and_apply_impulse(
		const entity_handle recoil_body, 
		const float scale, 
		const bool angular_impulse = false, 
		const float additional_angle = 0.f,
		const bool positional_impulse = false, 
		const float positional_rotation = 0.f
	);
	
	void cooldown(const float amount_ms);
};
