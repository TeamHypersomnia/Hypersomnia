#pragma once
#include "game/cosmos/entity_id_declaration.h"

template <class id_type>
struct basic_inventory_slot_id;

using inventory_slot_id = basic_inventory_slot_id<entity_id>;
using signi_inventory_slot_id = basic_inventory_slot_id<signi_entity_id>;
