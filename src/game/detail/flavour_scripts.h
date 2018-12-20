#pragma once
#include "game/detail/inventory/inventory_slot_types.h"
#include "game/cosmos/entity_flavour_id.h"

inventory_space_type calc_space_occupied_of_purchased(const cosmos&, const entity_flavour_id&);

