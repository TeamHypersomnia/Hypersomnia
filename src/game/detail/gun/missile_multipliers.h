#pragma once
#include "game/components/gun_component.h"
#include "game/components/missile_component.h"

/*
	What a fired round takes from the gun it was fired from.
*/

inline void set_missile_multipliers_of_gun(components::missile& missile, const invariants::gun& gun_def) {
	missile.power_multiplier_of_sender = gun_def.damage_multiplier;
	missile.headshot_multiplier_of_sender = gun_def.headshot_multiplier;
	missile.head_radius_multiplier_of_sender = gun_def.head_radius_multiplier;
	missile.decal_scale_of_sender = gun_def.gunshot_decal_scale;
}
