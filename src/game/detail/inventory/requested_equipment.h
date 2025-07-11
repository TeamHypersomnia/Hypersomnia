#pragma once
#include <cstdint>
#include "game/organization/special_flavour_id_types.h"
#include "augs/misc/simple_pair.h"
#include "game/cosmos/step_declaration.h"
#include "game/detail/spells/all_spells.h"

using other_equipment_vector = augs::constant_size_vector<augs::simple_pair<uint32_t, item_flavour_id>, 10>;

class cosmos;

class allocate_new_entity_access;

struct requested_equipment {
	// GEN INTROSPECTOR struct requested_equipment
	item_flavour_id weapon;
	item_flavour_id weapon_secondary;
	item_flavour_id non_standard_mag;
	item_flavour_id non_standard_charge;
	int num_given_ammo_pieces = -1;

	item_flavour_id back_wearable;
#if 0
	item_flavour_id over_back_wearable;
#endif
	item_flavour_id belt_wearable;
	item_flavour_id personal_deposit_wearable;
	item_flavour_id shoulder_wearable;
	item_flavour_id armor_wearable;

	other_equipment_vector other_equipment;

	learnt_spells_array_type spells_to_give = {};
	bool perform_recoils = true;

	float haste_time = 0.0f;
	// END GEN INTROSPECTOR

	bool has_weapon(entity_flavour_id) const;

	template <class E>
	entity_id generate_for(
		allocate_new_entity_access access,
		const E& character, 
		logic_step step,
		int max_effects_played = 2
	) const;

private:
	template <class E, class F>
	entity_id generate_for_impl(
		allocate_new_entity_access access,
		const E& character, 
		cosmos& cosm,
		F&& on_step,
		int max_effects_played
	) const;

};
