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
basic_inventory_slot_handle<C>::basic_inventory_slot_handle(owner_reference owner, inventory_slot_id raw_id) : owner(owner), raw_id(raw_id) {

}

template <bool C>
typename basic_inventory_slot_handle<C>::owner_reference basic_inventory_slot_handle<C>::get_cosmos() const {
	return owner;
}

template <bool C>
typename basic_inventory_slot_handle<C>::entity_handle_type basic_inventory_slot_handle<C>::get_handle() const {
	return make_handle(raw_id.container_entity);
}

template <bool C>
typename basic_inventory_slot_handle<C>::entity_handle_type basic_inventory_slot_handle<C>::make_handle(entity_id id) const {
	return owner[id];
}

template <bool C>
typename basic_inventory_slot_handle<C> basic_inventory_slot_handle<C>::make_handle(inventory_slot_id id) const {
	return owner[id];
}

template <bool C>
typename basic_inventory_slot_handle<C>::slot_pointer basic_inventory_slot_handle<C>::operator->() const {
	return &get_handle().get<components::container>().slots[raw_id.type];
}

template <bool C>
typename basic_inventory_slot_handle<C>::slot_reference basic_inventory_slot_handle<C>::operator*() const {
	return *operator->();
}

template <bool C>
typename basic_inventory_slot_handle<C>::slot_reference basic_inventory_slot_handle<C>::get() const {
	return *operator->();
}

template <bool C>
bool basic_inventory_slot_handle<C>::alive() const {
	if (get_handle().dead())
		return false;

	const auto* container = get_handle().find<components::container>();

	return container && container->slots.find(raw_id.type) != container->slots.end();
}

template <bool C>
bool basic_inventory_slot_handle<C>::dead() const {
	return !alive();
}

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
typename basic_inventory_slot_handle<C>::entity_handle_type basic_inventory_slot_handle<C>::try_get_item() const {
	return make_handle(has_items() ? (*this).get_items_inside()[0] : entity_id());
}

template <bool C>
bool basic_inventory_slot_handle<C>::is_empty_slot() const {
	return alive() && get().items_inside.size() == 0;
}

template <bool C>
bool basic_inventory_slot_handle<C>::should_item_inside_keep_physical_body(entity_id until_parent) const {
	bool should_item_here_keep_physical_body = get().is_physical_attachment_slot;

	if (get_handle() == until_parent) {
		return should_item_here_keep_physical_body;
	}

	auto* maybe_item = get_handle().find<components::item>();

	if (maybe_item) {
		//if (maybe_item->current_slot.get_handle().alive() && maybe_item->current_slot.get_handle() == until_parent)
		//	return should_item_here_keep_physical_body;
		//else 
		auto slot = owner[maybe_item->current_slot];

		if (slot.alive())
			return std::min(should_item_here_keep_physical_body, slot.should_item_inside_keep_physical_body(until_parent));
	}

	return should_item_here_keep_physical_body;
}

template <bool C>
float basic_inventory_slot_handle<C>::calculate_density_multiplier_due_to_being_attached() const {
	ensure(get().is_physical_attachment_slot);
	float density_multiplier = get().attachment_density_multiplier;

	auto* maybe_item = get_handle().find<components::item>();

	
	if (maybe_item) {
		auto slot = owner.get_handle(maybe_item->current_slot);

		if (slot.alive())
			return density_multiplier * slot.calculate_density_multiplier_due_to_being_attached();
	}


	return density_multiplier;
}

template <bool C>
components::transform basic_inventory_slot_handle<C>::sum_attachment_offsets_of_parents(entity_id attached_item) const {
	auto attached_item_handle = make_handle(attached_item);

	auto offset = get().attachment_offset;

	auto sticking = get().attachment_sticking_mode;

	offset.pos += attached_item_handle.get<components::fixtures>().get_aabb_size().get_sticking_offset(sticking);
	offset.pos += get_handle().get<components::fixtures>().get_aabb_size().get_sticking_offset(sticking);

	offset += attached_item_handle.get<components::item>().attachment_offsets_per_sticking_mode[sticking];

	auto* maybe_item = get_handle().find<components::item>();

	if (maybe_item && make_handle(maybe_item->current_slot).alive())
		return offset + make_handle(maybe_item->current_slot).sum_attachment_offsets_of_parents(get_handle());

	return offset;
}

template <bool C>
typename basic_inventory_slot_handle<C>::entity_handle_type basic_inventory_slot_handle<C>::get_root_container() const {
	auto* maybe_item = get_handle().find<components::item>();

	if (maybe_item && make_handle(maybe_item->current_slot).alive())
		return make_handle(maybe_item->current_slot).get_root_container();

	return get_handle();
}

template <bool C>
std::vector<typename basic_inventory_slot_handle<C>::entity_handle_type> basic_inventory_slot_handle<C>::get_items_inside() const {
	return get_cosmos()[get().items_inside];
}

template <bool C>
typename basic_inventory_slot_handle<C>::entity_handle_type basic_inventory_slot_handle<C>::get_container() const {
	return make_handle(raw_id.container_entity);
}

template <bool C>
unsigned basic_inventory_slot_handle<C>::calculate_free_space_with_parent_containers() const {
	auto maximum_space = calculate_free_space_with_children();

	auto* maybe_item = get_handle().find<components::item>();

	if (maybe_item && make_handle(maybe_item->current_slot).alive())
		return std::min(maximum_space, make_handle(maybe_item->current_slot).calculate_free_space_with_parent_containers());

	return maximum_space;
}

template <bool C>
bool basic_inventory_slot_handle<C>::can_contain(entity_id id) const {
	if (dead())
		return false;

	basic_item_slot_transfer_request<C> r(get_cosmos()[id], *this);
	return containment_result(r).transferred_charges > 0;
}

template <bool C>
inventory_slot_id basic_inventory_slot_handle<C>::get_id() const {
	return raw_id;
}

template <bool C>
basic_inventory_slot_handle<C>::operator inventory_slot_id() const {
	return get_id();
}

template <bool C>
basic_inventory_slot_handle<C>::operator basic_inventory_slot_handle<true>() const {
	return basic_inventory_slot_handle<true>(owner, raw_id);
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
	if (get().has_unlimited_space())
		return 1000000 * SPACE_ATOMS_PER_UNIT;

	unsigned space = get().space_available;

	for (auto e : get_items_inside()) {
		auto occupied = calculate_space_occupied_with_children(e);
		ensure(occupied <= space);
		space -= occupied;
	}

	return space;
}

template <bool C>
maybe_const_ref_t<C, slot_button> basic_inventory_slot_handle<C>::get_button() const {
	return get().button;
}


template class basic_inventory_slot_handle<true>;
template class basic_inventory_slot_handle<false>;