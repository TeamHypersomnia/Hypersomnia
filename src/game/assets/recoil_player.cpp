#include "recoil_player.h"

vec2 recoil_player_instance::shoot_and_get_impulse(const recoil_player& meta) {
	const auto result = meta.offsets[index++];
	index %= meta.offsets.size();
	return result;
}
