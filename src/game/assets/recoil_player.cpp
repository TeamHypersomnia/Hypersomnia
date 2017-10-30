#include "augs/log.h"
#include "augs/misc/randomization.h"

#include "game/assets/recoil_player.h"

real32 recoil_player_instance::shoot_and_get_impulse(const recoil_player& meta) {
	const auto heat = current_heat;
	current_heat += heat_per_shot;

	const auto index = static_cast<std::size_t>(heat);

	if (index >= meta.offsets.size()) {
		fast_randomization rng{static_cast<rng_seed_type>(heat * 100)};
		return rng.randval(meta.fallback_random_magnitude);
	}

	return meta.offsets[index];
}

void recoil_player_instance::cooldown(const real32 amount_ms) {
#if 0
	if (current_heat > 0) {
		DEBUG_LOG("%x", current_heat);
	}
#endif

	current_heat -= std::min(current_heat, cooldown_speed * amount_ms);
}
