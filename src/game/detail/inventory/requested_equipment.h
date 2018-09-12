#pragma once
#include "game/cosmos/entity_flavour_id.h"
#include "augs/misc/simple_pair.h"

struct requested_ammo {
	// GEN INTROSPECTOR struct requested_ammo
	entity_flavour_id magazine;
	entity_flavour_id charge;
	// END GEN INTROSPECTOR
};

using other_equipment_vector = std::vector<augs::simple_pair<int, entity_flavour_id>>;
using spare_ammo_vector = std::vector<augs::simple_pair<int, requested_ammo>>;

struct requested_equipment {
	// GEN INTROSPECTOR struct requested_equipment
	entity_flavour_id weapon;
	requested_ammo weapon_ammo;
	entity_flavour_id backpack;
	entity_flavour_id belt_wearable;
	entity_flavour_id personal_deposit_wearable;

	other_equipment_vector other_equipment;
	spare_ammo_vector spare_mags;
	// END GEN INTROSPECTOR
};
