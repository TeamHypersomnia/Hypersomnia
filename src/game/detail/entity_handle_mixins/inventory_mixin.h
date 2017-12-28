#pragma once
#include "game/detail/inventory/inventory_slot_handle_declaration.h"
#include "game/transcendental/entity_handle_declaration.h"
#include "game/transcendental/step_declaration.h"
#include "game/transcendental/entity_id.h"
#include "augs/build_settings/platform_defines.h"
#include "game/detail/inventory/wielding_result.h"
#include "augs/callback_result.h"

template<bool is_const, class entity_handle_type>
class basic_inventory_mixin {
	typedef basic_inventory_slot_handle<is_const> inventory_slot_handle_type;
public:
	static constexpr size_t hand_count = 2;
	typedef std::array<entity_id, hand_count> hand_selections_array;

	entity_handle_type get_owning_transfer_capability() const;
	bool owning_transfer_capability_alive_and_same_as_of(const entity_id) const;

	inventory_slot_handle_type determine_holstering_slot_for(const entity_handle_type holstered_item) const;
	inventory_slot_handle_type determine_pickup_target_slot_for(const entity_handle_type picked_item) const;
	
	inventory_slot_handle_type get_current_slot() const;

	inventory_slot_handle_type get_first_free_hand() const;
	
	inventory_slot_handle_type get_primary_hand() const;
	inventory_slot_handle_type get_secondary_hand() const;

	inventory_slot_handle_type get_hand_no(const size_t) const;
	entity_handle_type get_if_any_item_in_hand_no(const size_t) const;

	template <class F>
	void for_each_hand(F callback) const {
		for (size_t i = 0; i < hand_count; ++i) {
			const auto hand = get_hand_no(i);

			if (hand.alive()) {
				if (callback(hand) == callback_result::ABORT) {
					return;
				}
			}
		}
	}

	augs::constant_size_vector<entity_id, 2> get_wielded_guns() const;
	augs::constant_size_vector<entity_id, 2> get_wielded_items() const;

	inventory_item_address get_address_from_root(const entity_id until = entity_id()) const;

	wielding_result make_wielding_transfers_for(const hand_selections_array) const;

	wielding_result swap_wielded_items() const;

private:
	template <class S, class I>
	callback_result for_each_contained_slot_and_item_recursive(
		S slot_callback, 
		I item_callback, 
		inventory_traversal& trav
	) const {
		const auto this_item_handle = *static_cast<const entity_handle_type*>(this);
		auto& cosm = this_item_handle.get_cosmos();

		if (this_item_handle.template has<components::container>()) {
			trav.current_address.directions.push_back(slot_function());
			const auto this_item_attachment_offset = trav.attachment_offset;
			const bool does_this_item_remain_physical = trav.item_remains_physical;

			for (const auto& s : this_item_handle.template get<components::container>().slots) {
				const auto this_slot_id = inventory_slot_id(s.first, this_item_handle.get_id());
				
				const auto slot_callback_result = slot_callback(cosm[this_slot_id]);

				if (slot_callback_result == recursive_callback_result::ABORT) {
					return callback_result::ABORT;
				}
				else if (slot_callback_result == recursive_callback_result::CONTINUE_DONT_RECURSE) {
					continue;
				}
				else if (slot_callback_result == recursive_callback_result::CONTINUE_AND_RECURSE) {
					const bool is_this_slot_physical = does_this_item_remain_physical && s.second.makes_physical_connection();

					for (const auto& id : get_items_inside(this_item_handle, s.first)) {
						const auto child_item_handle = cosm[id];
						trav.parent_slot = this_slot_id;
						trav.current_address.directions.back() = this_slot_id.type;
						trav.item_remains_physical = is_this_slot_physical;

						if (trav.item_remains_physical) {
							trav.attachment_offset =
								this_item_attachment_offset + get_attachment_offset(
									s.second,
									this_item_attachment_offset,
									child_item_handle
								)
							;
						}

						const auto item_callback_result = item_callback(child_item_handle, static_cast<const inventory_traversal&>(trav));

						if (item_callback_result == recursive_callback_result::ABORT) {
							return callback_result::ABORT;
						}
						else if (item_callback_result == recursive_callback_result::CONTINUE_DONT_RECURSE) {
							continue;
						}
						else if (item_callback_result == recursive_callback_result::CONTINUE_AND_RECURSE) {
							if (child_item_handle.for_each_contained_slot_and_item_recursive(
								slot_callback,
								item_callback,
								trav
							) == callback_result::ABORT) {
								return callback_result::ABORT;
							}
						}
						else {
							ensure(false && "bad recursive_callback_result");
						}
					}
				}
				else {
					ensure(false && "bad recursive_callback_result");
				}
			}
		}

		return callback_result::CONTINUE;
	}

public:

	template <class S, class I>
	void for_each_contained_slot_and_item_recursive(
		S&& slot_callback, 
		I&& item_callback
	) const {
		const auto this_item_handle = *static_cast<const entity_handle_type*>(this);

		inventory_traversal trav;
		trav.current_address.root_container = this_item_handle.get_id();
		
		for_each_contained_slot_and_item_recursive(
			std::forward<S>(slot_callback),
			std::forward<I>(item_callback),
			trav
		);
	}

	template <class I>
	void for_each_contained_item_recursive(I&& item_callback) const {
		for_each_contained_slot_and_item_recursive(
			[](auto) { return recursive_callback_result::CONTINUE_AND_RECURSE; }, 
			std::forward<I>(item_callback)
		);
	}

	template <class S>
	void for_each_contained_slot_recursive(S&& slot_callback) const {
		for_each_contained_slot_and_item_recursive(
			std::forward<S>(slot_callback), 
			[](auto...) { return recursive_callback_result::CONTINUE_AND_RECURSE; }
		);
	}
};

template<bool, class>
class inventory_mixin;

template<class entity_handle_type>
class inventory_mixin<false, entity_handle_type> : public basic_inventory_mixin<false, entity_handle_type> {
public:

};

template<class entity_handle_type>
class inventory_mixin<true, entity_handle_type> : public basic_inventory_mixin<true, entity_handle_type> {
};
