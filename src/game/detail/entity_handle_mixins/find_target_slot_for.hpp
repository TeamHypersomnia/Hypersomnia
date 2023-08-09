#pragma once
#include "game/detail/entity_handle_mixins/inventory_mixin.hpp"
#include "game/detail/inventory/item_mounting.hpp"
#include "game/detail/weapon_like.h"
#include "game/detail/explosive/like_explosive.h"

template <class E>
template <class F>
int inventory_mixin<E>::count_contained(F predicate) const {
	int n = 0;

	for_each_contained_item_recursive(
		[&](const auto& typed_item) {
			if (predicate(typed_item)) {
				++n;
			}
		},
		std::nullopt
	);

	return n;
}

template <class E>
int inventory_mixin<E>::count_contained(const item_flavour_id& id) const {
	return count_contained(
		[&id](const auto& typed_item) { 
			return item_flavour_id(typed_item.get_flavour_id()) == id; 
		}
	);
}

template <class E>
template <class F>
void inventory_mixin<E>::for_each_candidate_slot(
	const candidate_holster_types& types,
	F&& callback
) const {
	const auto& searched_root_container = *static_cast<const E*>(this);

	for (const auto& t : types) {
		if (t == candidate_holster_type::WEARABLES) {
			callback(searched_root_container[slot_function::BELT]);
			callback(searched_root_container[slot_function::BACK]);
			callback(searched_root_container[slot_function::OVER_BACK]);
			callback(searched_root_container[slot_function::SHOULDER]);
			callback(searched_root_container[slot_function::TORSO_ARMOR]);
			callback(searched_root_container[slot_function::HAT]);
			callback(searched_root_container[slot_function::PERSONAL_DEPOSIT]);
		}
		else if (t == candidate_holster_type::HANDS) {
			callback(searched_root_container[slot_function::PRIMARY_HAND]);
			callback(searched_root_container[slot_function::SECONDARY_HAND]);
		}
		else if (t == candidate_holster_type::CONTAINERS) {
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

			if (const auto back_slot = searched_root_container[slot_function::OVER_BACK]) {
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
   	const candidate_holster_types& types,
	const slot_finding_opts& opts
) const {
	const auto& searched_root_container = *static_cast<const E*>(this);
	(void)searched_root_container;

	auto& cosm = item.get_cosmos();

	ensure(item.alive()) 
	ensure(searched_root_container.alive());
	ensure(entity_id(item.get_id()) != entity_id(searched_root_container.get_id()));

	inventory_slot_id target_slot;

	auto check_slot = [&](const auto slot) {
		if (slot.dead() || target_slot.is_set()) {
			return;
		}

		if (opts.test(slot_finding_opt::OMIT_MOUNTED_SLOTS)) {
			if (slot->is_mounted_slot()) {
				return;
			}
		}

		if (opts.test(slot_finding_opt::ALL_CHARGES_MUST_FIT)) {
			if (slot.can_contain_whole(item)) {
				target_slot = slot;
			}
		}
		else {
			if (slot.can_contain(item)) {
				target_slot = slot;
			}
		}
	};

	for_each_candidate_slot(
		types,
		check_slot
	);

	return cosm[target_slot];
}

template <class E>
template <class handle_type>
typename inventory_mixin<E>::inventory_slot_handle_type inventory_mixin<E>::find_holstering_slot_for(const handle_type holstered_item) const {
	return find_slot_for(holstered_item, { candidate_holster_type::WEARABLES, candidate_holster_type::CONTAINERS }, { slot_finding_opt::ALL_CHARGES_MUST_FIT, slot_finding_opt::OMIT_MOUNTED_SLOTS } );
}

template <class E>
template <class handle_type>
typename inventory_mixin<E>::inventory_slot_handle_type inventory_mixin<E>::find_pickup_target_slot_for(const handle_type picked_item, const slot_finding_opts opts) const {
	auto finding_order = candidate_holster_types { 
		candidate_holster_type::WEARABLES, 
		candidate_holster_type::CONTAINERS, 
		candidate_holster_type::HANDS 
	};

	{
		if (::is_weapon_like(picked_item) && !::is_like_plantable_bomb(picked_item)) {
			/* If it is a weapon, try to hold them in hands before trying containers. */

			const auto& searched_root_container = *static_cast<const E*>(this);

			if (searched_root_container.get_wielded_items().empty()) {
				/* But only if we don't hold anything to not break our wielding. */
				std::swap(finding_order.front(), finding_order.back());
			}
		}
	}

	return find_slot_for(picked_item, finding_order, opts);
}
