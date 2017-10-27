#include "inventory_mixin.h"

#include "game/transcendental/entity_handle.h"
#include "game/detail/inventory/inventory_slot_id.h"
#include "game/detail/inventory/inventory_slot_handle.h"
#include "game/detail/inventory/item_slot_transfer_request.h"
#include "game/detail/inventory/inventory_utils.h"
#include "game/components/item_component.h"
#include "game/components/item_slot_transfers_component.h"
#include "game/components/gun_component.h"
#include "game/components/sentience_component.h"
#include "game/transcendental/cosmos.h"

template <bool C, class D>
D basic_inventory_mixin<C, D>::get_owning_transfer_capability() const {
	const auto& self = *static_cast<const D*>(this);
	auto& cosmos = self.get_cosmos();

	if (self.dead()) {
		return cosmos[entity_id()];
	}

	if (self.template has<components::item_slot_transfers>()) {
		return self;
	}

	const auto* const maybe_item = self.template find<components::item>();

	if (!maybe_item || cosmos[maybe_item->current_slot].dead()) {
		return cosmos[entity_id()];
	}

	return cosmos[maybe_item->current_slot].get_container().get_owning_transfer_capability();
}

template <bool C, class D>
bool basic_inventory_mixin<C, D>::owning_transfer_capability_alive_and_same_as_of(const entity_id b) const {
	const auto& self = *static_cast<const D*>(this);
	auto& cosmos = self.get_cosmos();
	const auto this_capability = get_owning_transfer_capability();
	const auto b_capability = cosmos[b].get_owning_transfer_capability();
	
	return this_capability.alive() && b_capability.alive() && this_capability == b_capability;
}

template <bool C, class D>
typename basic_inventory_mixin<C, D>::inventory_slot_handle_type basic_inventory_mixin<C, D>::get_primary_hand() const {
	const auto& self = *static_cast<const D*>(this);
	auto& cosmos = self.get_cosmos();
	ensure(self.template has<components::sentience>());

	return self[slot_function::PRIMARY_HAND];
}

template <bool C, class D>
typename basic_inventory_mixin<C, D>::inventory_slot_handle_type basic_inventory_mixin<C, D>::get_secondary_hand() const {
	const auto& self = *static_cast<const D*>(this);
	auto& cosmos = self.get_cosmos();
	ensure(self.template has<components::sentience>());

	return self[slot_function::SECONDARY_HAND];
}

template <bool C, class D>
typename basic_inventory_mixin<C, D>::inventory_slot_handle_type basic_inventory_mixin<C, D>::get_hand_no(const size_t index) const {
	const auto& self = *static_cast<const D*>(this);

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

template <bool C, class D>
D basic_inventory_mixin<C, D>::get_if_any_item_in_hand_no(const size_t index) const {
	const auto& self = *static_cast<const D*>(this);
	const auto hand = self.get_hand_no(index);
	
	entity_id item;

	if (hand.alive()) {
		item = hand.get_item_if_any();
	}
	
	return self.get_cosmos()[item];
}

template <bool C, class D>
typename basic_inventory_mixin<C, D>::inventory_slot_handle_type basic_inventory_mixin<C, D>::get_first_free_hand() const {
	const auto& self = *static_cast<const D*>(this);
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

template <bool C, class D>
typename basic_inventory_mixin<C, D>::inventory_slot_handle_type basic_inventory_mixin<C, D>::get_current_slot() const {
	const auto& self = *static_cast<const D*>(this);
	
	const auto* const maybe_item = self.template find<components::item>();

	if (maybe_item == nullptr) {
		return self.get_cosmos()[inventory_slot_id()];
	}

	return self.get_cosmos()[maybe_item->current_slot];
}

template <bool C, class D>
inventory_item_address basic_inventory_mixin<C, D>::get_address_from_root(const entity_id until) const {
	const auto& self = *static_cast<const D*>(this);
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

template <bool C, class D>
typename basic_inventory_mixin<C, D>::inventory_slot_handle_type basic_inventory_mixin<C, D>::determine_holstering_slot_for(const D holstered_item) const {
	const auto& searched_root_container = *static_cast<const D*>(this);
	auto& cosmos = holstered_item.get_cosmos();

	ensure(holstered_item.alive()) 
	ensure(searched_root_container.alive());
	ensure(holstered_item != searched_root_container);

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

template <bool C, class D>
typename basic_inventory_mixin<C, D>::inventory_slot_handle_type basic_inventory_mixin<C, D>::determine_pickup_target_slot_for(const D picked_item) const {
	const auto& searched_root_container = *static_cast<const D*>(this);
	
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

template <bool C, class D>
augs::constant_size_vector<entity_id, 2> basic_inventory_mixin<C, D>::get_wielded_guns() const {
	const auto& self = *static_cast<const D*>(this);
	auto result = self.get_wielded_items();
	
	erase_if(
		result, 
		[&](const auto item) {
			return !self.get_cosmos()[item].template has<components::gun>();
		}
	);

	return result;
}

template <bool C, class D>
augs::constant_size_vector<entity_id, 2> basic_inventory_mixin<C, D>::get_wielded_items() const {
	const auto& self = *static_cast<const D*>(this);
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

template <bool C, class D>
wielding_result basic_inventory_mixin<C, D>::swap_wielded_items() const {
	const auto& self = *static_cast<const D*>(this);
	auto& cosmos = self.get_cosmos();
	
	wielding_result result;

	const bool both_hands_available = self.get_hand_no(0).alive() && self.get_hand_no(1).alive();

	if (both_hands_available) {
		const auto in_primary = self.get_if_any_item_in_hand_no(0);
		const auto in_secondary = self.get_if_any_item_in_hand_no(1);

		auto& transfers = result.transfers;

		if (in_primary.alive() && in_secondary.alive()) {
			transfers = swap_slots_for_items(in_primary, in_secondary);
		}
		else if (in_primary.alive()) {
			transfers.push_back({ in_primary, self.get_secondary_hand() });
		}
		else if (in_secondary.alive()) {
			transfers.push_back({ in_secondary, self.get_primary_hand() });
		}

		result.result = wielding_result::type::SUCCESSFUL;
	}

	return result;
}

template <bool C, class D>
wielding_result basic_inventory_mixin<C, D>::make_wielding_transfers_for(const hand_selections_array selections) const {
	const auto& self = *static_cast<const D*>(this);
	auto& cosmos = self.get_cosmos();

	wielding_result result;
	result.result = wielding_result::type::THE_SAME_SETUP;

	augs::constant_size_vector<item_slot_transfer_request, hand_count> holsters;
	augs::constant_size_vector<item_slot_transfer_request, hand_count> draws;

	for (size_t i = 0; i < selections.size(); ++i) {
		const auto hand = self.get_hand_no(i);

		if (hand.dead()) {
			continue;
		}

		const auto item_for_hand = cosmos[selections[i]];
		const auto item_in_hand = hand.get_item_if_any();

		const bool identical_outcome =
			item_in_hand.dead() && item_for_hand.dead()
			|| item_in_hand == item_for_hand
		;

		if (identical_outcome) {
			continue;
		}

		if (item_in_hand.alive()) {
			const auto holstering_slot = self.determine_holstering_slot_for(item_in_hand);

			if (holstering_slot.dead()) {
				result.result = wielding_result::type::NO_SPACE_FOR_HOLSTER;
				break;
			}

			holsters.push_back({ item_in_hand, holstering_slot });
		}

		if (item_for_hand.alive()) {
			draws.push_back({ item_for_hand, hand });
		}

		result.result = wielding_result::type::SUCCESSFUL;
	}

	concatenate(result.transfers, holsters);
	concatenate(result.transfers, draws);

	return result;
}

// explicit instantiation
template class basic_inventory_mixin<false, basic_entity_handle<false>>;
template class basic_inventory_mixin<true, basic_entity_handle<true>>;
template class inventory_mixin<false, basic_entity_handle<false>>;
template class inventory_mixin<true, basic_entity_handle<true>>;
