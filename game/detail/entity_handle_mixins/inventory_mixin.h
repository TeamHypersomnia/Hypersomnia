#pragma once
#include "game/detail/inventory_slot_handle_declaration.h"
#include "game/transcendental/entity_handle_declaration.h"
#include "game/transcendental/step_declaration.h"

template<bool is_const, class entity_handle_type>
class basic_inventory_mixin {
	typedef basic_inventory_slot_handle<is_const> inventory_slot_handle_type;
public:

	entity_handle_type get_owning_transfer_capability() const;

	inventory_slot_handle_type determine_hand_holstering_slot_for(const entity_handle_type holstered_item) const;
	inventory_slot_handle_type determine_pickup_target_slot_for(const entity_handle_type picked_item) const;
	
	inventory_slot_handle_type get_current_slot() const;

	inventory_slot_handle_type first_free_hand() const;
	
	inventory_slot_handle_type map_primary_action_to_secondary_hand_if_primary_empty(const bool is_action_secondary) const;
	
	std::vector<entity_handle_type> guns_wielded() const;

	inventory_item_address get_address_from_root() const;

	bool wields_in_primary_hand(const const_entity_handle what_item) const;
	bool wields_in_secondary_hand(const const_entity_handle what_item) const;

private:
	template <class S, class I>
	void for_each_contained_slot_and_item_recursive(S slot_callback, I item_callback, inventory_traversal& trav) const {
		const auto this_item_handle = *static_cast<const entity_handle_type*>(this);
		maybe_const_ref_t<is_const, cosmos> cosm = this_item_handle.get_cosmos();

		if (this_item_handle.has<components::container>()) {
			trav.current_address.directions.push_back(slot_function());
			const auto this_item_attachment_offset = trav.attachment_offset;
			const bool this_item_remains_physical = trav.item_remains_physical;

			for (const auto& s : this_item_handle.get<components::container>().slots) {
				const auto this_slot_id = inventory_slot_id(s.first, this_item_handle.get_id());
				
				slot_callback(cosm[this_slot_id]);
				
				const bool this_slot_physical = this_item_remains_physical && s.second.is_physical_attachment_slot;

				for (const auto& id : s.second.items_inside) {
					const auto child_item_handle = cosm[id];
					trav.parent_slot = this_slot_id;
					trav.current_address.directions.back() = this_slot_id.type;
					trav.item_remains_physical = this_slot_physical;

					if (trav.item_remains_physical) {
						trav.attachment_offset = this_item_attachment_offset + get_attachment_offset(s.second, this_item_attachment_offset, child_item_handle);
					}

					item_callback(child_item_handle, static_cast<const inventory_traversal&>(trav));
					child_item_handle.for_each_contained_slot_and_item_recursive(slot_callback, item_callback, trav);
				}
			}
		}
	}

public:

	template <class S, class I>
	void for_each_contained_slot_and_item_recursive(S slot_callback, I item_callback) const {
		const auto this_item_handle = *static_cast<const entity_handle_type*>(this);

		inventory_traversal trav;
		trav.current_address.root_container = this_item_handle.get_id();
		for_each_contained_slot_and_item_recursive(slot_callback, item_callback, trav);
	}

	template <class F>
	void for_each_contained_item_recursive(F callback) const {
		for_each_contained_slot_and_item_recursive([](auto){}, callback);
	}
};

template<bool, class>
class inventory_mixin;

template<class entity_handle_type>
class inventory_mixin<false, entity_handle_type> : public basic_inventory_mixin<false, entity_handle_type> {
public:

	bool wield_in_hands(
		logic_step& step,
		const entity_id first = entity_id(), 
		const entity_id second = entity_id()
	) const;
};

template<class entity_handle_type>
class inventory_mixin<true, entity_handle_type> : public basic_inventory_mixin<true, entity_handle_type> {
};
