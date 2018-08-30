#pragma once
#include "game/detail/entity_handle_mixins/inventory_mixin.h"
#include "game/detail/entity_handle_mixins/get_owning_transfer_capability.hpp"
#include "game/components/item_sync.h"

template <class E>
std::optional<unsigned> inventory_mixin<E>::find_space_occupied() const {
	const auto& self = *static_cast<const E*>(this);

	if (const auto item = self.template find<components::item>()) {
		return item->get_charges() * self.template get<invariants::item>().space_occupied_per_charge;
	}

	return std::nullopt;
}

template <class E>
int inventory_mixin<E>::num_charges_fitting_in(const inventory_slot_handle_type& where) const {
	const auto& self = *static_cast<const E*>(this);

	if (const auto per_charge = self.template get<invariants::item>().space_occupied_per_charge; per_charge != 0) {
		const auto free_space = where.calc_real_space_available();

		return free_space / per_charge;
	}

	return 0;
}

template <class E>
void inventory_mixin<E>::set_charges(const int n) const {
	const auto& self = *static_cast<const E*>(this);

	self.template get<components::item>().set_charges(n);
}

template <class E>
typename inventory_mixin<E>::generic_handle_type inventory_mixin<E>::get_topmost_container() const {
	const auto& self = *static_cast<const E*>(this);
	auto& cosmos = self.get_cosmos();

	if (self.dead()) {
		return cosmos[entity_id()];
	}

	if (self.template has<components::item_slot_transfers>()) {
		return self;
	}

	if (const auto item = self.template find<components::item>()) {
		if (const auto slot = cosmos[item->get_current_slot()]) {
			return slot.get_container().get_topmost_container();
		}
	}

	if (self.template has<invariants::container>()) {
		return self;
	}

	return cosmos[entity_id()];
}

template <class E>
bool inventory_mixin<E>::owning_transfer_capability_alive_and_same_as_of(const entity_id b) const {
	const auto& self = *static_cast<const E*>(this);
	auto& cosmos = self.get_cosmos();
	const auto this_capability = get_owning_transfer_capability();
	const auto b_capability = cosmos[b].get_owning_transfer_capability();

	return this_capability.alive() && b_capability.alive() && this_capability == b_capability;
}

template <class E>
typename inventory_mixin<E>::inventory_slot_handle_type inventory_mixin<E>::get_primary_hand() const {
	const auto& self = *static_cast<const E*>(this);
	return self[slot_function::PRIMARY_HAND];
}

template <class E>
typename inventory_mixin<E>::inventory_slot_handle_type inventory_mixin<E>::get_secondary_hand() const {
	const auto& self = *static_cast<const E*>(this);
	return self[slot_function::SECONDARY_HAND];
}

template <class E>
typename inventory_mixin<E>::inventory_slot_handle_type inventory_mixin<E>::get_hand_no(const std::size_t index) const {
	const auto& self = *static_cast<const E*>(this);

	if (index == 0) {
		return get_primary_hand();
	}
	else if (index == 1) {
		return get_secondary_hand();
	}
	else {
		ensure(false && "bad hand index");
		return self.get_cosmos()[inventory_slot_id()];
	}
}

template <class E>
typename inventory_mixin<E>::generic_handle_type inventory_mixin<E>::get_if_any_item_in_hand_no(const std::size_t index) const {
	const auto& self = *static_cast<const E*>(this);
	const auto hand = self.get_hand_no(index);

	entity_id item;

	if (hand.alive()) {
		item = hand.get_item_if_any();
	}

	return self.get_cosmos()[item];
}

template <class E>
hand_action inventory_mixin<E>::calc_hand_action(const std::size_t requested_index) const {
	const auto& self = *static_cast<const E*>(this);

	const auto in_primary = self.get_if_any_item_in_hand_no(0);
	const auto in_secondary = self.get_if_any_item_in_hand_no(1);

	if (in_primary.alive() && in_secondary.alive()) {
		return {
			requested_index,
			(requested_index == 0 ? in_primary : in_secondary).get_id(),
			false
		};
	}
	else if (in_primary.alive()) {
		return {
			requested_index,
			in_primary.get_id(),
			requested_index == 1
		};
	}
	else if (in_secondary.alive()) {
		return {
			requested_index,
			in_secondary.get_id(),
			requested_index == 1
		};
	}

	return { static_cast<std::size_t>(-1), entity_id(), false };
}

template <class E>
bool inventory_mixin<E>::only_secondary_holds_item() const {
	const auto& self = *static_cast<const E*>(this);
	return self.get_if_any_item_in_hand_no(0).dead() && self.get_if_any_item_in_hand_no(1).alive();
}

template <class E>
typename inventory_mixin<E>::inventory_slot_handle_type inventory_mixin<E>::get_first_free_hand() const {
	const auto& self = *static_cast<const E*>(this);
	auto& cosmos = self.get_cosmos();

	inventory_slot_id target_slot;

	self.for_each_hand(
		[&](const auto hand) {
			if (hand.is_empty_slot()) {
				target_slot = hand;

				return callback_result::ABORT;
			}

			return callback_result::CONTINUE;
		}
	);

	return cosmos[target_slot];
}

template <class E>
inventory_item_address inventory_mixin<E>::get_address_from_root(const entity_id until) const {
	const auto& self = *static_cast<const E*>(this);
	auto& cosmos = self.get_cosmos();

	inventory_item_address output;
	inventory_slot_id current_slot = get_current_slot();

	while (cosmos[current_slot].alive()) {
		output.root_container = current_slot.container_entity;
		output.directions.push_back(current_slot.type);

		if (until == current_slot.container_entity) {
			break;
		}

		current_slot = cosmos[current_slot.container_entity].get_current_slot();
	}

	std::reverse(output.directions.begin(), output.directions.end());

	return output;
}

template <class E>
augs::constant_size_vector<entity_id, 2> inventory_mixin<E>::get_wielded_guns() const {
	const auto& self = *static_cast<const E*>(this);
	auto result = self.get_wielded_items();

	erase_if(
		result, 
		[&](const auto item) {
			return !self.get_cosmos()[item].template has<components::gun>();
		}
	);

	return result;
}

template <class E>
augs::constant_size_vector<entity_id, 2> inventory_mixin<E>::get_wielded_items() const {
	const auto& self = *static_cast<const E*>(this);
	augs::constant_size_vector<entity_id, 2> result;

	self.for_each_hand(
		[&](const auto hand) {
			const auto wielded = hand.get_item_if_any();

			if (wielded.alive()) {
				result.push_back(wielded);
			}

			return callback_result::CONTINUE;
		}
	);

	return result;
}

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
				target_slot = slot;
				return recursive_callback_result::ABORT;
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

template <class E>
template <class F>
void inventory_mixin<E>::for_each_hand(F callback) const {
	for (std::size_t i = 0; i < hand_count; ++i) {
		const auto hand = get_hand_no(i);

		if (hand.alive()) {
			if (callback(hand) == callback_result::ABORT) {
				return;
			}
		}
	}
}
