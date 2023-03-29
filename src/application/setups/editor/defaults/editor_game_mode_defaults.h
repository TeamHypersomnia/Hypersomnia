#pragma once
#include "application/setups/editor/editor_official_resource_map.hpp"

inline void setup_game_mode_defaults(
	editor_game_mode_resource_editable& e,
	const editor_official_resource_map& o
) {
	e.playtesting.equipment.metropolis.firearm = o[test_shootable_weapons::SZTURM];
	e.playtesting.equipment.metropolis.melee = o[test_melee_weapons::ELECTRIC_RAPIER];
	e.playtesting.equipment.metropolis.backpack = true;

	e.playtesting.equipment.resistance.firearm = o[test_shootable_weapons::BAKA47];
	e.playtesting.equipment.resistance.melee = o[test_melee_weapons::ELECTRIC_RAPIER];
	e.playtesting.equipment.resistance.backpack = true;

	e.bomb_defusal.warmup_equipment.metropolis.firearm = o[test_shootable_weapons::ELON_HRL];
	e.bomb_defusal.warmup_equipment.metropolis.melee = o[test_melee_weapons::POSEIDON];
	e.bomb_defusal.warmup_equipment.metropolis.extra_ammo_pieces = 3;
	e.bomb_defusal.warmup_equipment.metropolis.backpack = true;

	e.bomb_defusal.warmup_equipment.resistance.firearm = o[test_shootable_weapons::ELON_HRL];
	e.bomb_defusal.warmup_equipment.resistance.melee = o[test_melee_weapons::FURY_THROWER];
	e.bomb_defusal.warmup_equipment.resistance.extra_ammo_pieces = 3;
	e.bomb_defusal.warmup_equipment.resistance.backpack = true;
}
