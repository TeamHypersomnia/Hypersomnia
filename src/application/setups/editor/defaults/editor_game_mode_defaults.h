#pragma once
#include "application/setups/editor/editor_official_resource_map.hpp"

inline void setup_game_mode_defaults(
	editor_game_mode_resource_editable& e,
	const editor_official_resource_map& o
) {
	e.quick_test.equipment.metropolis.firearm = o[test_shootable_weapons::SZTURM];
	e.quick_test.equipment.metropolis.melee = o[test_melee_weapons::ELECTRIC_RAPIER];
	e.quick_test.equipment.metropolis.backpack = true;

	e.quick_test.equipment.resistance.firearm = o[test_shootable_weapons::BAKA47];
	e.quick_test.equipment.resistance.melee = o[test_melee_weapons::ELECTRIC_RAPIER];
	e.quick_test.equipment.resistance.backpack = true;

	e.bomb_defusal.warmup_equipment.metropolis.firearm = o[test_shootable_weapons::ELON];
	e.bomb_defusal.warmup_equipment.metropolis.melee = o[test_melee_weapons::POSEIDON];
	e.bomb_defusal.warmup_equipment.metropolis.extra_ammo_pieces = 3;
	e.bomb_defusal.warmup_equipment.metropolis.backpack = true;

	e.bomb_defusal.warmup_equipment.resistance.firearm = o[test_shootable_weapons::ELON];
	e.bomb_defusal.warmup_equipment.resistance.melee = o[test_melee_weapons::FURY_THROWER];
	e.bomb_defusal.warmup_equipment.resistance.extra_ammo_pieces = 3;
	e.bomb_defusal.warmup_equipment.resistance.backpack = true;

	e.bomb_defusal.round_start_equipment.metropolis.firearm = o[test_shootable_weapons::SN69];
	e.bomb_defusal.round_start_equipment.metropolis.melee = o[test_melee_weapons::CYAN_SCYTHE];

	e.bomb_defusal.round_start_equipment.resistance.firearm = o[test_shootable_weapons::KEK9];
	e.bomb_defusal.round_start_equipment.resistance.melee = o[test_melee_weapons::YELLOW_DAGGER];

	e.gun_game.progression = {
		o[test_shootable_weapons::ELON],
		o[test_shootable_weapons::BULLDUP2000],

		o[test_shootable_weapons::DATUM],
		o[test_shootable_weapons::BAKA47],
		o[test_shootable_weapons::SZTURM],
		o[test_shootable_weapons::BILMER2000],
		o[test_shootable_weapons::GALILEA],

		o[test_shootable_weapons::PRO90],
		o[test_shootable_weapons::CYBERSPRAY],
		o[test_shootable_weapons::ZAMIEC],

		o[test_shootable_weapons::WARX],
		o[test_shootable_weapons::GRADOBICIE],

		o[test_shootable_weapons::AWKA],
		o[test_shootable_weapons::HUNTER],

		o[test_shootable_weapons::DEAGLE],
		o[test_shootable_weapons::COVERT],
		o[test_shootable_weapons::AO44],
		o[test_shootable_weapons::CALICO],
		o[test_shootable_weapons::BULWARK],

		o[test_shootable_weapons::KEK9],
		o[test_shootable_weapons::SN69]
	};

	editor_requested_equipment basic_eq;
	basic_eq.melee = o[test_melee_weapons::ELECTRIC_RAPIER];
	basic_eq.backpack = true;
	basic_eq.extra_ammo_pieces = 6;

	/* To coincide with the spawn protection */
	basic_eq.haste_time = 3;

	editor_requested_equipment final_eq;
	final_eq.electric_armor = true;
	final_eq.melee = o[test_melee_weapons::YELLOW_DAGGER];
	final_eq.haste_time = 600;

	e.gun_game.basic_equipment[faction_type::METROPOLIS] = basic_eq;
	e.gun_game.basic_equipment[faction_type::RESISTANCE] = basic_eq;

	e.gun_game.final_equipment[faction_type::METROPOLIS] = final_eq;
	e.gun_game.final_equipment[faction_type::RESISTANCE] = final_eq;

	editor_requested_equipment warmup_eq;
	warmup_eq.melee = o[test_melee_weapons::ELECTRIC_RAPIER];
	warmup_eq.explosive = o[test_hand_explosives::FORCE_GRENADE];
	warmup_eq.num_explosives = 3;

	e.gun_game.warmup_equipment.resistance = warmup_eq;
	e.gun_game.warmup_equipment.metropolis = warmup_eq;
}
