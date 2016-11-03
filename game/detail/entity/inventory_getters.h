#pragma once
#include "game/detail/inventory_slot_handle_declaration.h"
#include "game/transcendental/entity_handle_declaration.h"

template<bool is_const, class entity_handle_type>
class inventory_getters {
	typedef basic_inventory_slot_handle<is_const> inventory_slot_handle_type;
public:

	entity_handle_type get_owning_transfer_capability() const;

	inventory_slot_handle_type determine_hand_holstering_slot_in(const entity_handle_type searched_root_container) const;
	inventory_slot_handle_type determine_pickup_target_slot_in(const entity_handle_type searched_root_container) const;
	
	inventory_slot_handle_type get_current_slot() const;

	inventory_slot_handle_type first_free_hand() const;
	
	inventory_slot_handle_type map_primary_action_to_secondary_hand_if_primary_empty(const bool is_action_secondary) const;
	
	std::vector<entity_handle_type> guns_wielded() const;

	template <class S, class I>
	void for_each_contained_slot_and_item_recursive(S slot_callback, I item_callback) const {
		const auto this_item_handle = *static_cast<const entity_handle_type*>(this);
		maybe_const_ref_t<is_const, cosmos> cosm = this_item_handle.get_cosmos();

		if (this_item_handle.has<components::container>()) {
			for (const auto& s : this_item_handle.get<components::container>().slots) {
				slot_callback(cosm[inventory_slot_id(s.first, this_item_handle.get_id())]);

				for (const auto& id : s.second.items_inside) {
					const auto child_handle = cosm[id];

					item_callback(child_handle);
					child_handle.for_each_contained_slot_and_item_recursive(slot_callback, item_callback);
				}
			}
		}
	}

	template <class F>
	void for_each_contained_item_recursive(F callback) const {
		for_each_contained_slot_and_item_recursive([](auto){}, callback);
	}
};
