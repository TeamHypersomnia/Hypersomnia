#pragma once
#include "game/detail/inventory_slot_handle_declaration.h"
#include "game/entity_handle_declaration.h"

template<bool is_const>
class inventory_getters {
	typedef typename basic_inventory_slot_handle<is_const> inventory_slot_handle_type;
	typedef basic_entity_handle<is_const> entity_handle_type;
public:

	entity_handle_type get_owning_transfer_capability() const;

	inventory_slot_handle_type determine_hand_holstering_slot(entity_handle_type searched_root_container) const;
	inventory_slot_handle_type determine_pickup_target_slot_in(entity_handle_type searched_root_container) const;
	
	inventory_slot_handle_type first_free_hand() const;
	
	inventory_slot_handle_type map_primary_action_to_secondary_hand_if_primary_empty(int is_action_secondary) const;
	
	std::vector<entity_handle_type> guns_wielded() const;
};