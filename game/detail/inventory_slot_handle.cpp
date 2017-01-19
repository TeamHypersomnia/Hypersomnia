#include "inventory_slot.h"
#include "inventory_slot_handle.h"
#include "game/transcendental/entity_handle.h"
#include "game/detail/item_slot_transfer_request.h"
#include "game/components/transform_component.h"
#include "game/components/container_component.h"
#include "game/components/fixtures_component.h"
#include "game/components/item_component.h"

#include "inventory_utils.h"
#include "game/transcendental/cosmos.h"

#include "game/detail/item_transfer_result.h"

template <bool C>
void basic_inventory_slot_handle<C>::unset() {
	raw_id.unset();
}

template <bool C>
bool basic_inventory_slot_handle<C>::is_hand_slot() const {
	return raw_id.type == slot_function::PRIMARY_HAND || raw_id.type == slot_function::SECONDARY_HAND;
}

template <bool C>
bool basic_inventory_slot_handle<C>::is_input_enabling_slot() const {
	return is_hand_slot();
}

template <bool C>
bool basic_inventory_slot_handle<C>::has_items() const {
	return alive() && get().items_inside.size() > 0;
}

template <bool C>
typename basic_inventory_slot_handle<C>::entity_handle_type basic_inventory_slot_handle<C>::get_item_if_any() const {
	return get_cosmos()[(has_items() ? (*this).get_items_inside()[0] : entity_id())];
}

template <bool C>
bool basic_inventory_slot_handle<C>::is_empty_slot() const {
	return alive() && get().items_inside.size() == 0;
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
std::vector<typename basic_inventory_slot_handle<C>::entity_handle_type> basic_inventory_slot_handle<C>::get_items_inside() const {
	return get_cosmos()[get().items_inside];
}

template <bool C>
typename basic_inventory_slot_handle<C>::entity_handle_type basic_inventory_slot_handle<C>::get_container() const {
	return get_cosmos()[raw_id.container_entity];
}

template <bool C>
unsigned basic_inventory_slot_handle<C>::calculate_free_space_with_parent_containers() const {
	const auto maximum_space = calculate_free_space_with_children();

	const auto* const maybe_item = get_container().find<components::item>();

	if (maybe_item && get_cosmos()[maybe_item->current_slot].alive()) {
		return std::min(maximum_space, get_cosmos()[maybe_item->current_slot].calculate_free_space_with_parent_containers());
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
std::vector<typename basic_inventory_slot_handle<C>::entity_handle_type> basic_inventory_slot_handle<C>::get_mounted_items() const {
	// TODO: actually implement mounted items
	return get_items_inside();

	//for (auto& i : items_inside) {
	//	auto handle = cosmos[i];
	//
	//	if (handle.get<components::item>().is_mounted())
	//		output.push_back(i);
	//}
	//
	//return output;
}

template <bool C>
unsigned basic_inventory_slot_handle<C>::calculate_free_space_with_children() const {
	if (get().has_unlimited_space()) {
		return 1000000 * SPACE_ATOMS_PER_UNIT;
	}

	unsigned space = get().space_available;

	for (const auto e : get_items_inside()) {
		const auto occupied = calculate_space_occupied_with_children(e);
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