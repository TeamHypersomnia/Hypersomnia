#include "recoil_player.h"

#include <cstddef>
#include <random>
#include <algorithm>

#include "augs/misc/randomization.h"
#include "augs/log.h"

float recoil_player_instance::shoot_and_get_impulse(const recoil_player& meta) {
	const auto heat = current_heat;
	current_heat += heat_per_shot;

	const auto index = std::size_t(heat);

	if(index >= meta.offsets.size()) {
		fast_randomization rng{std::size_t(heat * 100)};
		return rng.randval(meta.fallback_random_magnitude);
	}

	return meta.offsets[index];
}

void recoil_player_instance::cooldown(const float amount_ms) {
	if(current_heat > 0) DEBUG_LOG("%x", current_heat);
	current_heat -= std::min(current_heat, cooldown_speed * amount_ms);
}
