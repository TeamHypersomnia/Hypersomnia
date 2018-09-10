#pragma once
#include "game/detail/entity_handle_mixins/inventory_mixin.hpp"
#include "game/detail/inventory/item_mounting.hpp"

template <class E>
template <class handle_type>
typename inventory_mixin<E>::inventory_slot_handle_type inventory_mixin<E>::determine_holstering_slot_for(const handle_type holstered_item) const {
	const auto& searched_root_container = *static_cast<const E*>(this);
	auto& cosmos = holstered_item.get_cosmos();

	ensure(holstered_item.alive()) 
	ensure(searched_root_container.alive());
	ensure(holstered_item.get_id() != searched_root_container.get_id());

	inventory_slot_id target_slot;

	searched_root_container.for_each_contained_slot_recursive(
		[&](const auto slot) {
			if (slot.get_container() == holstered_item) {
				return recursive_callback_result::CONTINUE_DONT_RECURSE;
			}

			if (!slot.is_hand_slot() && slot.can_contain(holstered_item)) {
				if (mounting_conditions_type::NO_MOUNTING_REQUIRED == calc_mounting_conditions(holstered_item, slot)) {
					target_slot = slot;
					return recursive_callback_result::ABORT;
				}
			}

			return recursive_callback_result::CONTINUE_AND_RECURSE;
		}
	);

	return cosmos[target_slot];
}

template <class E>
template <class handle_type>
typename inventory_mixin<E>::inventory_slot_handle_type inventory_mixin<E>::determine_pickup_target_slot_for(const handle_type picked_item) const {
	const auto& searched_root_container = *static_cast<const E*>(this);

	ensure(picked_item.alive());
	ensure(searched_root_container.alive());

	auto& cosmos = picked_item.get_cosmos();

	inventory_slot_id target_slot;

	const auto holster_slot = searched_root_container.determine_holstering_slot_for(picked_item);

	if (holster_slot.alive()) {
		target_slot = holster_slot;
	}
	else {
		searched_root_container.for_each_hand(
			[&](const auto hand) {
				if (hand.can_contain(picked_item)) {
					target_slot = hand;

					return callback_result::ABORT;
				}

				return callback_result::CONTINUE;
			}
		);
	}

	return cosmos[target_slot];
}
