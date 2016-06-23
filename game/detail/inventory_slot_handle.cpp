#include "inventory_slot.h"
#include "inventory_slot_handle.h"
#include "game/entity_handle.h"
#include "game/detail/item_slot_transfer_request.h"
#include "game/components/transform_component.h"
#include "game/components/container_component.h"
#include "game/components/fixtures_component.h"
#include "game/components/item_component.h"

#include "inventory_utils.h"
#include "game/cosmos.h"

#include "game/detail/item_transfer_result.h"

template <bool C>
basic_inventory_slot_handle<C>::basic_inventory_slot_handle(owner_reference owner, inventory_slot_id raw_id) : owner(owner), raw_id(raw_id) {

}

template <bool C>
typename basic_inventory_slot_handle<C>::entity_handle_type basic_inventory_slot_handle<C>::get_handle() const {
	return make_handle(raw_id.container_entity);
}

template <bool C>
typename basic_inventory_slot_handle<C>::entity_handle_type basic_inventory_slot_handle<C>::make_handle(entity_id id) const {
	return owner.get_handle(id);
}

template <bool C>
typename basic_inventory_slot_handle<C> basic_inventory_slot_handle<C>::make_handle(inventory_slot_id id) const {
	return owner.get_handle(id);
}

template <bool C>
typename basic_inventory_slot_handle<C>::slot_pointer basic_inventory_slot_handle<C>::operator->() const {
	return &get_handle().get<components::container>().slots.at(raw_id.type);
}

template <bool C>
typename basic_inventory_slot_handle<C>::slot_reference basic_inventory_slot_handle<C>::operator*() const {
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
	return alive() && (*this)->items_inside.size() > 0;
}

template <bool C>
typename basic_inventory_slot_handle<C>::entity_handle_type basic_inventory_slot_handle<C>::try_get_item() const {
	return make_handle(has_items() ? (*this)->items_inside[0] : entity_id());
}

template <bool C>
bool basic_inventory_slot_handle<C>::is_empty_slot() const {
	return alive() && (*this)->items_inside.size() == 0;
}

template <bool C>
bool basic_inventory_slot_handle<C>::should_item_inside_keep_physical_body(entity_id until_parent) const {
	bool should_item_here_keep_physical_body = (*this)->is_physical_attachment_slot;

	if (get_handle() == until_parent) {
		return should_item_here_keep_physical_body;
	}

	auto* maybe_item = get_handle().find<components::item>();

	if (maybe_item) {
		//if (maybe_item->current_slot.get_handle().alive() && maybe_item->current_slot.get_handle() == until_parent)
		//	return should_item_here_keep_physical_body;
		//else 
		auto slot = owner.get_handle(maybe_item->current_slot);

		if (slot.alive())
			return std::min(should_item_here_keep_physical_body, slot.should_item_inside_keep_physical_body(until_parent));
	}

	return should_item_here_keep_physical_body;
}

template <bool C>
float basic_inventory_slot_handle<C>::calculate_density_multiplier_due_to_being_attached() const {
	ensure((*this)->is_physical_attachment_slot);
	float density_multiplier = (*this)->attachment_density_multiplier;

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

	auto offset = (*this)->attachment_offset;

	auto sticking = (*this)->attachment_sticking_mode;

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
typename basic_inventory_slot_handle<C>::entity_handle_type basic_inventory_slot_handle<C>::get_container() const {
	return make_handle(raw_id.container_entity);
}

template <bool C>
unsigned basic_inventory_slot_handle<C>::calculate_free_space_with_parent_containers() const {
	auto maximum_space = (*this)->calculate_free_space_with_children();

	auto* maybe_item = get_handle().find<components::item>();

	if (maybe_item && make_handle(maybe_item->current_slot).alive())
		return std::min(maximum_space, make_handle(maybe_item->current_slot).calculate_free_space_with_parent_containers());

	return maximum_space;
}


template <bool C>
void basic_inventory_slot_handle<C>::for_each_descendant(std::function<void(entity_handle_type item)> f) const {
	for (auto& i : (*this)->items_inside) {
		auto handle = make_handle(i);

		f(handle);

		auto* container = handle.find<components::container>();

		if (container)
			for (auto& s : container->slots)
				handle[s.first].for_each_descendant(f);
	}
}

template <bool C>
bool basic_inventory_slot_handle<C>::can_contain(entity_id id) const {
	if (dead())
		return false;

	item_slot_transfer_request r;
	r.target_slot = *this;
	r.item = id;

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

template class basic_inventory_slot_handle<true>;
template class basic_inventory_slot_handle<false>;