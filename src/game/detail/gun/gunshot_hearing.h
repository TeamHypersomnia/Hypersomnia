#pragma once
#include "game/components/gun_component.h"

/*
	How far a gunshot is heard - by the bots and on the minimap alike.
*/

inline real32 calc_gunshot_hearing_distance(const invariants::gun& gun_def) {
	return gun_def.muzzle_shot_sound.modifier.max_distance * 0.8f;
}
