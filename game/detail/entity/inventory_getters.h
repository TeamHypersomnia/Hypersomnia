#pragma once
template<class entity_handle_type, class traits>
class inventory_getters {
	typedef typename traits::inventory_slot_handle_type inventory_slot_handle_type;
public:

	entity_handle_type get_owning_transfer_capability() const;

	inventory_slot_handle_type determine_hand_holstering_slot(entity_handle_type searched_root_container) const;
	inventory_slot_handle_type determine_pickup_target_slot_in(entity_handle_type searched_root_container) const;
	
	inventory_slot_handle_type first_free_hand() const;
	
	inventory_slot_handle_type map_primary_action_to_secondary_hand_if_primary_empty(int is_action_secondary) const;
	
	std::vector<entity_handle_type> guns_wielded() const;
};