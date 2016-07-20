#include "augs/ensure.h"
#include "recoil_player.h"
#include "game/components/physics_component.h"
#include "game/entity_handle.h"
#include "game/cosmos.h"

vec2 recoil_player::shoot_and_get_offset() {
	if (current_offset > int(offsets.size() - 1))
		reversed = true;
	ensure(repeat_last_n_offsets > 0 && repeat_last_n_offsets < offsets.size());
	if (current_offset + repeat_last_n_offsets <= offsets.size()) {
		reversed = false;
		delta_offset = 0;
	}

	if (reversed) {
		delta_offset++;
		current_offset = offsets.size() - 1 - delta_offset;
	}

	return offsets[current_offset++] * scale;
}

void recoil_player::shoot_and_apply_impulse(entity_handle recoil_body, float additional_scale, bool angular_impulse,
	float additional_angle, bool positional_impulse, float positional_rotation) {
	auto recoil_physics = recoil_body.get<components::physics>();

	auto offset = shoot_and_get_offset();

	if (angular_impulse) {
		recoil_physics.apply_angular_impulse(offset.radians() * additional_scale + additional_angle);
	}

	if (positional_impulse) {
		recoil_physics.apply_impulse((offset * additional_scale).rotate(positional_rotation, vec2()));
	}
}

void recoil_player::cooldown(double amount_ms) {
	remaining_cooldown_duration -= amount_ms;

	if (remaining_cooldown_duration < 0) {
		remaining_cooldown_duration = single_cooldown_duration_ms;

		if(current_offset > 0)
			--current_offset;
	}
}
