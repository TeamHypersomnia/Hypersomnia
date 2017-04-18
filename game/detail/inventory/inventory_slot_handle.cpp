#include "inventory_slot.h"
#include "inventory_slot_handle.h"
#include "game/transcendental/entity_handle.h"
#include "game/detail/inventory/item_slot_transfer_request.h"
#include "game/components/transform_component.h"
#include "game/components/container_component.h"
#include "game/components/fixtures_component.h"
#include "game/components/item_component.h"

#include "inventory_utils.h"
#include "game/transcendental/cosmos.h"

#include "game/detail/inventory/item_transfer_result.h"

template <bool C>
void basic_inventory_slot_handle<C>::unset() {
	raw_id.unset();
}

template <bool C>
bool basic_inventory_slot_handle<C>::is_hand_slot() const {
	return
		raw_id.type == slot_function::PRIMARY_HAND
		|| raw_id.type == slot_function::SECONDARY_HAND
	;
}

template <bool C>
size_t basic_inventory_slot_handle<C>::get_hand_index() const {
	if (raw_id.type == slot_function::PRIMARY_HAND) {
		return 0;
	}
	else if (raw_id.type == slot_function::SECONDARY_HAND) {
		return 1;
	}
	else {
		ensure(false);
		return 0xdeadbeef;
	}
}

template <bool C>
bool basic_inventory_slot_handle<C>::has_items() const {
	return get().items_inside.size() > 0;
}

template <bool C>
typename basic_inventory_slot_handle<C>::entity_handle_type basic_inventory_slot_handle<C>::get_item_if_any() const {
	return get_cosmos()[(has_items() ? (*this).get_items_inside()[0] : entity_id())];
}

template <bool C>
bool basic_inventory_slot_handle<C>::is_empty_slot() const {
	return get().items_inside.size() == 0;
}

template <bool C>
bool basic_inventory_slot_handle<C>::should_item_inside_keep_physical_body(const entity_id until_parent) const {
	const bool should_item_here_keep_physical_body = get().is_physical_attachment_slot;

	if (get_container() == until_parent) {
		return should_item_here_keep_physical_body;
	}

	const auto* const maybe_item = get_container().find<components::item>();

	if (maybe_item) {
		//if (maybe_item->current_slot.get_container().alive() && maybe_item->current_slot.get_container() == until_parent)
		//	return should_item_here_keep_physical_body;
		//else 
		const auto slot = owner[maybe_item->current_slot];

		if (slot.alive()) {
			return std::min(should_item_here_keep_physical_body, slot.should_item_inside_keep_physical_body(until_parent));
		}
	}

	return should_item_here_keep_physical_body;
}

template <bool C>
float basic_inventory_slot_handle<C>::calculate_density_multiplier_due_to_being_attached() const {
	ensure(get().is_physical_attachment_slot);
	
	const float density_multiplier = get().attachment_density_multiplier;

	const auto* const maybe_item = get_container().find<components::item>();
	
	if (maybe_item) {
		const auto slot = owner.get_handle(maybe_item->current_slot);

		if (slot.alive()) {
			return density_multiplier * slot.calculate_density_multiplier_due_to_being_attached();
		}
	}

	return density_multiplier;
}

template <bool C>
typename basic_inventory_slot_handle<C>::entity_handle_type basic_inventory_slot_handle<C>::get_root_container() const {
	const auto slot = get_container().get_current_slot();

	if (slot.alive()) {
		return slot.get_root_container();
	}

	return get_container();
}

template <bool C>
typename basic_inventory_slot_handle<C>::entity_handle_type basic_inventory_slot_handle<C>::get_root_container_until(const entity_id container_entity) const {
	const auto slot = get_container().get_current_slot();

	if (slot.alive() && slot.get_container() != container_entity) {
		return slot.get_root_container_until(container_entity);
	}

	return get_container();
}

template <bool C>
bool basic_inventory_slot_handle<C>::is_child_of(const entity_id container_entity) const {
	return get_container() == container_entity || get_root_container_until(container_entity) == container_entity;
}

template <bool C>
typename basic_inventory_slot_handle<C>::entity_handle_type basic_inventory_slot_handle<C>::get_container() const {
	return get_cosmos()[raw_id.container_entity];
}

template <bool C>
unsigned basic_inventory_slot_handle<C>::calculate_real_free_space() const {
	const auto maximum_space = calculate_local_free_space();

	const auto* const maybe_item = get_container().find<components::item>();

	if (maybe_item != nullptr && get_cosmos()[maybe_item->current_slot].alive()) {
		return std::min(maximum_space, get_cosmos()[maybe_item->current_slot].calculate_real_free_space());
	}

	return maximum_space;
}

template <bool C>
bool basic_inventory_slot_handle<C>::can_contain(const entity_id id) const {
	if (dead()) {
		return false;
	}

	return query_containment_result(get_cosmos()[id], *this).transferred_charges > 0;
}

template <bool C>
unsigned basic_inventory_slot_handle<C>::calculate_local_free_space() const {
	if (get().has_unlimited_space()) {
		return 1000000 * SPACE_ATOMS_PER_UNIT;
	}

	unsigned space = get().space_available;

	for (const auto e : get_items_inside()) {
		const auto occupied = calculate_space_occupied_with_children(get_cosmos()[e]);
		ensure(occupied <= space);
		space -= occupied;
	}

	return space;
}

//template <bool C>
//maybe_const_ref_t<C, slot_button> basic_inventory_slot_handle<C>::get_button() const {
//	return get().button;
//}


template class basic_inventory_slot_handle<true>;
template class basic_inventory_slot_handle<false>;