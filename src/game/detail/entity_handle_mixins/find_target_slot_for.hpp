#pragma once
#include "game/detail/entity_handle_mixins/inventory_mixin.hpp"
#include "game/detail/inventory/item_mounting.hpp"
#include "game/detail/weapon_like.h"

template <class E>
int inventory_mixin<E>::count_contained(const item_flavour_id& id) const {
	int n = 0;

	for_each_contained_item_recursive(
		[&](const auto& typed_item) {
			if (item_flavour_id(typed_item.get_flavour_id()) == id) {
				++n;
			}
		},
		std::nullopt
	);

	return n;
}

template <class E>
template <class F>
void inventory_mixin<E>::for_each_candidate_slot(
	const slot_finding_opts& opts,
	F&& callback
) const {
	const auto& searched_root_container = *static_cast<const E*>(this);

	for (const auto& o : opts) {
		if (o == slot_finding_opt::CHECK_WEARABLES) {
			callback(searched_root_container[slot_function::BELT]);
			callback(searched_root_container[slot_function::BACK]);
			callback(searched_root_container[slot_function::SHOULDER]);
			callback(searched_root_container[slot_function::TORSO_ARMOR]);
			callback(searched_root_container[slot_function::HAT]);
			callback(searched_root_container[slot_function::PERSONAL_DEPOSIT]);
		}
		else if (o == slot_finding_opt::CHECK_HANDS) {
			callback(searched_root_container[slot_function::PRIMARY_HAND]);
			callback(searched_root_container[slot_function::SECONDARY_HAND]);
		}
		else if (o == slot_finding_opt::CHECK_CONTAINERS) {
			if (const auto personal_slot = searched_root_container[slot_function::PERSONAL_DEPOSIT]) {
				if (const auto personal_wearable = personal_slot.get_item_if_any()) {
					callback(personal_wearable[slot_function::ITEM_DEPOSIT]);
				}
			}

			if (const auto back_slot = searched_root_container[slot_function::BACK]) {
				if (const auto back_wearable = back_slot.get_item_if_any()) {
					callback(back_wearable[slot_function::ITEM_DEPOSIT]);
				}
			}
		}
	}
}

template <class E>
template <class handle_type>
typename inventory_mixin<E>::inventory_slot_handle_type inventory_mixin<E>::find_slot_for(
	const handle_type item,
   	const slot_finding_opts& opts
) const {
	const auto& searched_root_container = *static_cast<const E*>(this);
	(void)searched_root_container;

	auto& cosm = item.get_cosmos();

	ensure(item.alive()) 
	ensure(searched_root_container.alive());
	ensure(item.get_id() != searched_root_container.get_id());

	inventory_slot_id target_slot;

	auto check_slot = [&](const auto slot) {
		if (slot.dead() || target_slot.is_set()) {
			return;
		}

		if (slot.can_contain(item)) {
			target_slot = slot;
		}
	};

	for_each_candidate_slot(
		opts,
		check_slot
	);

	return cosm[target_slot];
}

template <class E>
template <class handle_type>
typename inventory_mixin<E>::inventory_slot_handle_type inventory_mixin<E>::find_holstering_slot_for(const handle_type holstered_item) const {
	return find_slot_for(holstered_item, { slot_finding_opt::CHECK_WEARABLES, slot_finding_opt::CHECK_CONTAINERS });
}

template <class E>
template <class handle_type>
typename inventory_mixin<E>::inventory_slot_handle_type inventory_mixin<E>::find_pickup_target_slot_for(const handle_type picked_item) const {
	auto finding_order = slot_finding_opts { 
		slot_finding_opt::CHECK_WEARABLES, 
		slot_finding_opt::CHECK_CONTAINERS, 
		slot_finding_opt::CHECK_HANDS 
	};

	{
		if (is_weapon_like(picked_item)) {
			/* If it is a weapon, try to hold them in hands before trying containers. */

			const auto& searched_root_container = *static_cast<const E*>(this);

			if (searched_root_container.get_wielded_items().empty()) {
				/* But only if we don't hold anything to not break our wielding. */
				std::swap(finding_order.front(), finding_order.back());
			}
		}
	}

	return find_slot_for(picked_item, finding_order);
}
