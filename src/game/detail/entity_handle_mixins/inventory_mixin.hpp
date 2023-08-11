#pragma once
#include "game/detail/entity_handle_mixins/inventory_mixin.h"
#include "game/detail/entity_handle_mixins/get_owning_transfer_capability.hpp"
#include "game/components/item_sync.h"
#include "game/detail/weapon_like.h"

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
int inventory_mixin<E>::get_charges() const {
	const auto& self = *static_cast<const E*>(this);

	return self.template get<components::item>().get_charges();
}

template <class E>
typename inventory_mixin<E>::generic_handle_type inventory_mixin<E>::get_topmost_container() const {
	const auto& self = *static_cast<const E*>(this);
	auto& cosm = self.get_cosmos();

	if (self.dead()) {
		return cosm[entity_id()];
	}

	if (self.template has<components::item_slot_transfers>()) {
		return self;
	}

	if (const auto item = self.template find<components::item>()) {
		if (const auto slot = cosm[item->get_current_slot()]) {
			return slot.get_container().get_topmost_container();
		}
	}

	if (self.template has<invariants::container>()) {
		return self;
	}

	return cosm[entity_id()];
}

template <class E>
bool inventory_mixin<E>::owning_transfer_capability_alive_and_same_as_of(const entity_id b) const {
	const auto& self = *static_cast<const E*>(this);
	auto& cosm = self.get_cosmos();
	const auto this_capability = get_owning_transfer_capability();
	const auto b_capability = cosm[b].get_owning_transfer_capability();

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
hand_action calc_hand_action(
	const std::size_t requested_index,
	const E in_primary,
	const E in_secondary
) {
	if (in_primary.alive() && in_secondary.alive()) {
		return {
			requested_index,
			(requested_index == 0 ? in_primary : in_secondary).get_id(),
			weapon_action_type::PRIMARY
		};
	}
	else if (in_primary.alive()) {
		return {
			0u,
			in_primary.get_id(),
			requested_index == 1 ? weapon_action_type::SECONDARY : weapon_action_type::PRIMARY
		};
	}
	else if (in_secondary.alive()) {
		return {
			1u,
			in_secondary.get_id(),
			requested_index == 1 ? weapon_action_type::SECONDARY : weapon_action_type::PRIMARY
		};
	}

	return { static_cast<std::size_t>(-1), entity_id(), weapon_action_type::COUNT };
}

template <class E>
hand_action inventory_mixin<E>::calc_hand_action(const std::size_t requested_index) const {
	const auto& self = *static_cast<const E*>(this);

	const auto in_primary = self.get_if_any_item_in_hand_no(0);
	const auto in_secondary = self.get_if_any_item_in_hand_no(1);

	return ::calc_hand_action(requested_index, in_primary, in_secondary);
}

template <class E>
hand_action inventory_mixin<E>::calc_viable_hand_action(const std::size_t requested_index) const {
	const auto& self = *static_cast<const E*>(this);

	auto actionable_or_default = [&](const auto& candidate) {
		if (::is_weapon_like(candidate) || ::is_armor_like(candidate)) {
			return candidate;
		}

		return self.get_cosmos()[entity_id()];
	};

	const auto in_primary = actionable_or_default(self.get_if_any_item_in_hand_no(0));
	const auto in_secondary = actionable_or_default(self.get_if_any_item_in_hand_no(1));

	return ::calc_hand_action(requested_index, in_primary, in_secondary);
}


template <class E>
bool inventory_mixin<E>::only_secondary_holds_item() const {
	const auto& self = *static_cast<const E*>(this);
	return self.get_if_any_item_in_hand_no(0).dead() && self.get_if_any_item_in_hand_no(1).alive();
}

template <class E>
typename inventory_mixin<E>::inventory_slot_handle_type inventory_mixin<E>::get_first_free_hand() const {
	const auto& self = *static_cast<const E*>(this);
	auto& cosm = self.get_cosmos();

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

	return cosm[target_slot];
}

template <class E>
inventory_item_address inventory_mixin<E>::get_address_from_root(const entity_id until) const {
	const auto& self = *static_cast<const E*>(this);
	auto& cosm = self.get_cosmos();

	inventory_item_address output;
	inventory_slot_id slot_it = get_current_slot();

	while (cosm[slot_it].alive()) {
		output.root_container = slot_it.container_entity;
		output.directions.push_back(slot_it.type);

		if (until == slot_it.container_entity) {
			break;
		}

		slot_it = cosm[slot_it.container_entity].get_current_slot();
	}

	std::reverse(output.directions.begin(), output.directions.end());

	return output;
}

template <class E>
typename inventory_mixin<E>::generic_handle_type inventory_mixin<E>::get_wielded_other_than(const entity_id id) const {
	const auto& self = *static_cast<const E*>(this);
	auto& cosm = self.get_cosmos();

	for (const auto& i : get_wielded_items()) {
		if (i != id) {
			return cosm[i];
		}
	}

	return cosm[entity_id()];
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
augs::constant_size_vector<entity_id, 2> inventory_mixin<E>::get_wielded_melees() const {
	const auto& self = *static_cast<const E*>(this);
	auto result = self.get_wielded_items();

	erase_if(
		result, 
		[&](const auto item) {
			return !self.get_cosmos()[item].template has<components::melee>();
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
template <class F>
void inventory_mixin<E>::for_each_hand(F callback) const {
	for (std::size_t i = 0; i < hand_count_v; ++i) {
		const auto hand = get_hand_no(i);

		if (hand.alive()) {
			if (callback(hand) == callback_result::ABORT) {
				return;
			}
		}
	}
}

template <class E>
template <class handle_type>
wielding_type inventory_mixin<E>::get_wielding_of(const handle_type item) const {
	const auto& items = get_wielded_items();

	for (const auto& i : items) {
		if (i == item) {
			return items.size() == 1 ? wielding_type::SINGLE_WIELDED : wielding_type::DUAL_WIELDED;
		}
	}

	return wielding_type::NOT_WIELDED;
}

template <class E>
auto inventory_mixin<E>::find_mounting_progress() const {
	const auto& self = *static_cast<const E*>(this);
	auto& cosm = self.get_cosmos();

	const auto progress = mapped_or_nullptr(cosm.get_global_solvable().pending_item_mounts, self.get_id());

	if (progress) {
		if (!progress->is_due_to_be_erased()) {
			return progress;
		}
	}

	return decltype(progress)(nullptr);
}

template <class E>
void inventory_mixin<E>::infer_item_physics_recursive() const {
	const auto& self = *static_cast<const E*>(this);
	ensure(self);

	self.infer_colliders_from_scratch();

	self.for_each_contained_item_recursive([](const auto& h) {
		h.infer_colliders();	
	});
}

template <class E>
void inventory_mixin<E>::infer_change_of_current_slot() const {
	const auto& self = *static_cast<const E*>(this);
	self.infer_rigid_body();

	infer_item_physics_recursive();
}
