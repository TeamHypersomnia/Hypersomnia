#include "augs/misc/randomization.h"
#include "game/assets/recoil_player.h"

real32 recoil_player_instance::shoot_and_get_impulse(
	const recoil_player_instance_def& def,
	const recoil_player& meta
) {
	const auto index = static_cast<std::size_t>(pattern_progress);

	if (index >= meta.offsets.size()) {
		pattern_progress += def.pattern_progress_per_shot * 0.01;

		auto rng = randomization(static_cast<rng_seed_type>(pattern_progress * 1000));
		return rng.randval(-1.f, 1.f);
	}
	else
	{
		pattern_progress += def.pattern_progress_per_shot;
	}

	return meta.offsets[index];
}

void recoil_player_instance::cooldown(
	const recoil_player_instance_def& def,
	real32 dt_secs
) {
	pattern_progress = augs::damp(pattern_progress, dt_secs, def.pattern_progress_damping);
}
