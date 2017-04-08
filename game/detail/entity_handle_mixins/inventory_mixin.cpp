#include "game/transcendental/entity_handle.h"
#include "game/detail/inventory/inventory_slot_id.h"
#include "game/detail/inventory/inventory_slot_handle.h"
#include "game/detail/inventory/item_slot_transfer_request.h"
#include "game/detail/inventory/inventory_utils.h"
#include "game/components/item_component.h"
#include "game/components/item_slot_transfers_component.h"
#include "game/components/gun_component.h"
#include "game/transcendental/cosmos.h"

#include "inventory_mixin.h"

template <bool C, class D>
D basic_inventory_mixin<C, D>::get_owning_transfer_capability() const {
	const auto& self = *static_cast<const D*>(this);
	auto& cosmos = self.get_cosmos();

	if (self.dead()) {
		return cosmos[entity_id()];
	}

	if (self.has<components::item_slot_transfers>()) {
		return self;
	}

	const auto* const maybe_item = self.find<components::item>();

	if (!maybe_item || cosmos[maybe_item->current_slot].dead()) {
		return cosmos[entity_id()];
	}

	return cosmos[maybe_item->current_slot].get_container().get_owning_transfer_capability();
}

template <bool C, class D>
typename basic_inventory_mixin<C, D>::inventory_slot_handle_type basic_inventory_mixin<C, D>::first_free_hand() const {
	const auto& self = *static_cast<const D*>(this);
	auto& cosmos = self.get_cosmos();

	const auto maybe_primary = self[slot_function::PRIMARY_HAND];
	const auto maybe_secondary = self[slot_function::SECONDARY_HAND];

	if (maybe_primary.is_empty_slot()) {
		return maybe_primary;
	}

	if (maybe_secondary.is_empty_slot()) {
		return maybe_secondary;
	}

	return cosmos[inventory_slot_id()];
}

template <bool C, class D>
typename basic_inventory_mixin<C, D>::inventory_slot_handle_type basic_inventory_mixin<C, D>::get_current_slot() const {
	const auto& self = *static_cast<const D*>(this);
	
	const auto* const maybe_item = self.find<components::item>();

	if (maybe_item == nullptr) {
		return self.get_cosmos()[inventory_slot_id()];
	}

	return self.get_cosmos()[maybe_item->current_slot];
}

template <bool C, class D>
inventory_item_address basic_inventory_mixin<C, D>::get_address_from_root() const {
	const auto& self = *static_cast<const D*>(this);
	auto& cosmos = self.get_cosmos();

	inventory_item_address output;
	inventory_slot_id current_slot = get_current_slot();
		
	while (cosmos[current_slot].alive()) {
		output.root_container = current_slot.container_entity;
		output.directions.push_back(current_slot.type);

		current_slot = cosmos[current_slot.container_entity].get_current_slot();
	}

	std::reverse(output.directions.begin(), output.directions.end());
	
	return output;
}

template <bool C, class D>
bool basic_inventory_mixin<C, D>::wields_in_primary_hand(const const_entity_handle what_item) const {
	const auto& self = *static_cast<const D*>(this);
	auto& cosmos = self.get_cosmos();

	return self[slot_function::PRIMARY_HAND].get_item_if_any() == what_item;
}

template <bool C, class D>
bool basic_inventory_mixin<C, D>::wields_in_secondary_hand(const const_entity_handle what_item) const {
	const auto& self = *static_cast<const D*>(this);
	auto& cosmos = self.get_cosmos();

	return self[slot_function::SECONDARY_HAND].get_item_if_any() == what_item;
}

template <bool C, class D>
typename basic_inventory_mixin<C, D>::inventory_slot_handle_type basic_inventory_mixin<C, D>::determine_hand_holstering_slot_for(const D holstered_item) const {
	const auto& searched_root_container = *static_cast<const D*>(this);
	auto& cosmos = holstered_item.get_cosmos();

	if (holstered_item.dead()) {
		return cosmos[inventory_slot_id()];
	}

	ensure(searched_root_container.alive());

	const auto slot_checker = [&](const slot_function type) {
		const auto maybe_slot = searched_root_container[type];

		if (maybe_slot.alive()) {
			if (maybe_slot.can_contain(holstered_item)) {
				return maybe_slot;
			}
			else {
				const auto items_inside = maybe_slot.get_items_inside();

				for (const auto potential_container_item_id : items_inside) {
					const auto potential_container_item = cosmos[potential_container_item_id];

					const auto category = get_slot_with_compatible_category(holstered_item, potential_container_item);

					if (category != slot_function::INVALID) {
						const auto compatible_slot = potential_container_item[category];

						if (compatible_slot.can_contain(holstered_item)) {
							return compatible_slot;
						}
					}
				}
			}
		}

		return cosmos[inventory_slot_id()];
	};

	const slot_function slots_to_check[] = {
		slot_function::SHOULDER_SLOT,
		slot_function::PRIMARY_HAND,
		slot_function::SECONDARY_HAND,
		slot_function::TORSO_ARMOR_SLOT
	};

	for(const auto s : slots_to_check) {
		const auto maybe_slot = slot_checker(s);

		if (maybe_slot.alive()) {
			return maybe_slot;
		}
	}

	return cosmos[inventory_slot_id()];
}

template <bool C, class D>
typename basic_inventory_mixin<C, D>::inventory_slot_handle_type basic_inventory_mixin<C, D>::determine_pickup_target_slot_for(const D picked_item) const {
	const auto& searched_root_container = *static_cast<const D*>(this);
	
	ensure(picked_item.alive());
	ensure(searched_root_container.alive());
	
	auto& cosmos = picked_item.get_cosmos();

	const auto hidden_slot = searched_root_container.determine_hand_holstering_slot_for(picked_item);

	if (hidden_slot.alive()) {
		return hidden_slot;
	}

	if (searched_root_container[slot_function::PRIMARY_HAND].can_contain(picked_item)) {
		return searched_root_container[slot_function::PRIMARY_HAND];
	}

	if (searched_root_container[slot_function::SECONDARY_HAND].can_contain(picked_item)) {
		return searched_root_container[slot_function::SECONDARY_HAND];
	}

	return cosmos[inventory_slot_id()];
}

template <bool C, class D>
typename basic_inventory_mixin<C, D>::inventory_slot_handle_type basic_inventory_mixin<C, D>::map_primary_action_to_secondary_hand_if_primary_empty(const bool is_action_secondary) const {
	const auto& root_container = *static_cast<const D*>(this);

	const auto primary = root_container[slot_function::PRIMARY_HAND];
	const auto secondary = root_container[slot_function::SECONDARY_HAND];

	if (primary.is_empty_slot()) {
		return secondary;
	}
	else { 
		return is_action_secondary ? secondary : primary;
	}
}

template <bool C, class D>
augs::constant_size_vector<entity_id, 2> basic_inventory_mixin<C, D>::guns_wielded() const {
	const auto& subject = *static_cast<const D*>(this);
	augs::constant_size_vector<entity_id, 2> result;

	{
		const auto wielded = subject[slot_function::PRIMARY_HAND].get_item_if_any();

		if (wielded.alive() && wielded.has<components::gun>()) {
			result.push_back(wielded);
		}
	}

	{
		const auto wielded = subject[slot_function::SECONDARY_HAND].get_item_if_any();

		if (wielded.alive() && wielded.has<components::gun>()) {
			result.push_back(wielded);
		}
	}

	return result;
}

template <bool C, class D>
augs::constant_size_vector<entity_id, 2> basic_inventory_mixin<C, D>::items_wielded() const {
	const auto& subject = *static_cast<const D*>(this);
	augs::constant_size_vector<entity_id, 2> result;

	{
		const auto wielded = subject[slot_function::PRIMARY_HAND].get_item_if_any();

		if (wielded.alive()) {
			result.push_back(wielded);
		}
	}

	{
		const auto wielded = subject[slot_function::SECONDARY_HAND].get_item_if_any();

		if (wielded.alive()) {
			result.push_back(wielded);
		}
	}

	return result;
}

template <bool C, class D>
wielding_result basic_inventory_mixin<C, D>::swap_wielded_items() const {
	const auto& subject = *static_cast<const D*>(this);
	auto& cosmos = subject.get_cosmos();
	
	wielding_result result;

	const auto in_primary = subject[slot_function::PRIMARY_HAND].get_item_if_any();
	const auto in_secondary = subject[slot_function::SECONDARY_HAND].get_item_if_any();
	
	auto& transfers = result.transfers;

	if (in_primary.alive() && in_secondary.alive()) {
		transfers = swap_slots_for_items(in_primary, in_secondary);
	}
	else if (in_primary.alive()) {
		transfers.push_back({ in_primary, subject[slot_function::SECONDARY_HAND] });
	}
	else if (in_secondary.alive()) {
		transfers.push_back({ in_secondary, subject[slot_function::PRIMARY_HAND] });
	}

	result.result = wielding_result::type::SUCCESSFUL;
	
	return result;
}

template <bool C, class D>
wielding_result basic_inventory_mixin<C, D>::wield_in_hands(
	entity_id first, 
	entity_id second
) const {
	const auto& subject = *static_cast<const D*>(this);
	auto& cosmos = subject.get_cosmos();
	
	wielding_result result;

	const auto in_primary = subject[slot_function::PRIMARY_HAND].get_item_if_any();
	const auto in_secondary = subject[slot_function::SECONDARY_HAND].get_item_if_any();

	const bool is_only_secondary_possibly_chosen = cosmos[first].dead();

	if (is_only_secondary_possibly_chosen) {
		std::swap(first, second);
	}

	if ((in_primary == first && in_secondary == second) || (is_only_secondary_possibly_chosen && (first == in_primary))) {
		result.result = wielding_result::type::THE_SAME_SETUP;
		return result;
	}

	const auto primary_holstering_slot = subject.determine_hand_holstering_slot_for(in_primary);
	const auto secondary_holstering_slot = subject.determine_hand_holstering_slot_for(in_secondary);

	const bool something_wont_fit = 
		(in_primary.alive() && primary_holstering_slot.dead()) ||
		(in_secondary.alive() && secondary_holstering_slot.dead());

	if (something_wont_fit) {
		result.result = wielding_result::type::SOMETHING_WONT_FIT;
		return result;
	}

	auto& transfers = result.transfers;

	if (primary_holstering_slot.alive()) {
		transfers.push_back({ in_primary, primary_holstering_slot });
	}

	if (secondary_holstering_slot.alive()) {
		transfers.push_back({ in_secondary, secondary_holstering_slot });
	}

	const auto first_handle = cosmos[first];
	const auto second_handle = cosmos[second];

	if (first_handle.alive()) {
		transfers.push_back({ first_handle, subject[slot_function::PRIMARY_HAND] });
	}

	if (second_handle.alive()) {
		transfers.push_back({ second_handle, subject[slot_function::SECONDARY_HAND] });
	}

	result.result = wielding_result::type::SUCCESSFUL;
	return result;
}

// explicit instantiation
template class basic_inventory_mixin<false, basic_entity_handle<false>>;
template class basic_inventory_mixin<true, basic_entity_handle<true>>;
template class inventory_mixin<false, basic_entity_handle<false>>;
template class inventory_mixin<true, basic_entity_handle<true>>;
