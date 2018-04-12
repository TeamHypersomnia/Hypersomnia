#include "augs/misc/randomization.h"
#include "game/assets/recoil_player.h"

real32 recoil_player_instance::shoot_and_get_impulse(
	const recoil_player_instance_def& def,
	const recoil_player& meta
) {
	const auto heat = current_heat;
	current_heat += def.heat_per_shot;

	const auto index = static_cast<std::size_t>(heat);

	if (index >= meta.offsets.size()) {
		auto rng = randomization(static_cast<rng_seed_type>(index));
		return rng.randval(-1.f, 1.f);
	}

	return meta.offsets[index];
}

void recoil_player_instance::cooldown(
	const recoil_player_instance_def& def,
	real32 amount_ms
) {
	current_heat -= std::min(current_heat, def.heat_cooldown_per_ms * amount_ms);
}
