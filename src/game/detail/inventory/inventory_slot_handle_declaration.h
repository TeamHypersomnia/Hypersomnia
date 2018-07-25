#pragma once
#include "game/cosmos/entity_handle_declaration.h"

template <class> 
class basic_inventory_slot_handle;

using inventory_slot_handle = basic_inventory_slot_handle<entity_handle>;
using const_inventory_slot_handle = basic_inventory_slot_handle<const_entity_handle>;

struct inventory_item_address;
struct inventory_traversal;