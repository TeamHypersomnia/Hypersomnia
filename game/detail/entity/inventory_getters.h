#pragma once
#include "game/detail/inventory_slot_handle_declaration.h"
#include "game/transcendental/entity_handle_declaration.h"

template<bool is_const, class entity_handle_type>
class inventory_getters {
	typedef basic_inventory_slot_handle<is_const> inventory_slot_handle_type;
public:

	entity_handle_type get_owning_transfer_capability() const;

	inventory_slot_handle_type determine_hand_holstering_slot_in(entity_handle_type searched_root_container) const;
	inventory_slot_handle_type determine_pickup_target_slot_in(entity_handle_type searched_root_container) const;
	
	inventory_slot_handle_type first_free_hand() const;
	
	inventory_slot_handle_type map_primary_action_to_secondary_hand_if_primary_empty(int is_action_secondary) const;
	
	std::vector<entity_handle_type> guns_wielded() const;

	template <class F>
	void for_each_contained_item_recursive(F callback) const {
		auto& item = *static_cast<const entity_handle_type*>(this);
		auto& cosmos = item.get_cosmos();

		if (item.has<components::container>()) {
			for (auto& s : item.get<components::container>().slots) {
				auto item_handles = cosmos[s.second.items_inside];

				for (auto it : item_handles) {
					callback(it);
					it.for_each_contained_item_recursive(callback);
				}
			}
		}
	}
};
