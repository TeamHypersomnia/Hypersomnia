#pragma once
#include "game/organization/special_flavour_id_types.h"
#include "augs/misc/simple_pair.h"
#include "game/cosmos/step_declaration.h"
#include "game/detail/spells/all_spells.h"

using other_equipment_vector = std::vector<augs::simple_pair<int, item_flavour_id>>;

struct requested_equipment {
	// GEN INTROSPECTOR struct requested_equipment
	item_flavour_id weapon;
	item_flavour_id non_standard_mag;
	item_flavour_id non_standard_charge;
	int num_given_ammo_pieces = -1;

	item_flavour_id back_wearable;
	item_flavour_id belt_wearable;
	item_flavour_id personal_deposit_wearable;
	item_flavour_id shoulder_wearable;

	other_equipment_vector other_equipment;

	learnt_spells_array_type spells_to_give = {};
	// END GEN INTROSPECTOR

	template <class E>
	entity_id generate_for(
		const E& character, 
		const logic_step step
	) const;
};
