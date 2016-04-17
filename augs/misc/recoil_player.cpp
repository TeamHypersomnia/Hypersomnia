#include "recoil_player.h"

vec2 recoil_player::shoot_and_get_offset(augs::deterministic_timestamp current_time) {
	if (current_offset > int(offsets.size() - 1))
		reversed = true;
	if (delta_offset > delta_offset_maximum - 1) {
		reversed = false;
		delta_offset = 0;
	}

	if (reversed) {
		delta_offset++;
		current_offset = int(offsets.size() - 1) - delta_offset;
	}
	LOG("C: %c", current_offset);
	return offsets[current_offset++] * scale;
}

void recoil_player::cooldown(double amount_ms) {
	remaining_cooldown_duration -= amount_ms;

	if (remaining_cooldown_duration < 0) {
		remaining_cooldown_duration = single_cooldown_duration_ms;
		--current_offset;
	}

	current_offset = std::max(current_offset, 0);
}
