#include "recoil_player.h"

vec2 recoil_player::shoot_and_get_offset(augs::deterministic_timestamp current_time) {
	double delta = (current_time - since_last_shot).get_milliseconds();
	since_last_shot = current_time;

	int cooldowns = delta / single_cooldown_duration_ms;

	if (cooldowns > 0)
		current_offset -= cooldowns;

	current_offset = std::max(current_offset, 0);
	current_offset = std::min(current_offset, int(offsets.size() - 1));
	return offsets[current_offset++] * scale;
}
