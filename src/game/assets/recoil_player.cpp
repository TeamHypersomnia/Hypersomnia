#include "augs/misc/randomization.h"
#include "game/assets/recoil_player.h"

real32 recoil_player_instance::shoot_and_get_impulse(
	const recoil_player_instance_def& def,
	const recoil_player& meta
) {
	const auto index = static_cast<std::size_t>(pattern_progress);

	pattern_progress += def.pattern_progress_per_shot;

	if (index >= meta.offsets.size()) {
		auto rng = randomization(static_cast<rng_seed_type>(index));
		return rng.randval(-1.f, 1.f);
	}

	return meta.offsets[index];
}

void recoil_player_instance::cooldown(
	const recoil_player_instance_def& def,
	real32 dt_secs
) {
	pattern_progress = augs::damp(pattern_progress, dt_secs, def.pattern_progress_damping);
}
