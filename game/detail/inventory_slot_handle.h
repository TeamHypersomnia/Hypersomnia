#pragma once
#include "inventory_slot_handle_declaration.h"
#include "game/transcendental/entity_handle_declaration.h"
#include "game/transcendental/entity_id.h"
#include "game/enums/slot_function.h"
#include "game/detail/inventory_slot_id.h"

#include "augs/templates/maybe_const.h"

struct inventory_slot;

namespace components {
	struct transform;
}

class cosmos;

template <bool is_const>
class basic_inventory_slot_handle {
	typedef basic_entity_handle<is_const> entity_handle_type;
	
	typedef maybe_const_ref_t<is_const, cosmos> owner_reference;
	typedef maybe_const_ref_t<is_const, inventory_slot> slot_reference;
	typedef maybe_const_ptr_t<is_const, inventory_slot> slot_pointer;

public:
	static constexpr bool is_const_value = is_const;
	
	basic_inventory_slot_handle(owner_reference owner, const inventory_slot_id raw_id);
	
	std::vector<entity_handle_type> get_mounted_items() const;

	owner_reference get_cosmos() const;

	owner_reference owner;
	inventory_slot_id raw_id;

	void unset();

	template <class F>
	void for_each_descendant(F callback) const {
		for (const auto& handle : get_items_inside()) {
			callback(handle);

			const auto* container = handle.find<components::container>();

			if (container)
				for (const auto& s : container->slots)
					handle[s.first].for_each_descendant(callback);
		}
	}

	slot_reference get() const;
	slot_reference operator*() const;
	slot_pointer operator->() const;

	bool alive() const;
	bool dead() const;

	bool can_contain(const entity_id) const;

	entity_handle_type get_item_if_any() const;
	entity_handle_type get_container() const;
	entity_handle_type get_root_container() const;

	std::vector<entity_handle_type> get_items_inside() const;

	bool has_items() const;
	bool is_empty_slot() const;

	bool is_hand_slot() const;
	bool is_input_enabling_slot() const;

	float calculate_density_multiplier_due_to_being_attached() const;
	unsigned calculate_free_space_with_children() const;

	bool should_item_inside_keep_physical_body(const entity_id until_parent = entity_id()) const;

	unsigned calculate_free_space_with_parent_containers() const;

	inventory_slot_id get_id() const;
	operator inventory_slot_id() const;
	operator basic_inventory_slot_handle<true>() const;
};

template <bool C>
inline basic_inventory_slot_handle<C>::basic_inventory_slot_handle(owner_reference owner, const inventory_slot_id raw_id) : owner(owner), raw_id(raw_id) {}

template <bool C>
inline typename basic_inventory_slot_handle<C>::owner_reference basic_inventory_slot_handle<C>::get_cosmos() const {
	return owner;
}

template <bool C>
inline typename basic_inventory_slot_handle<C>::slot_pointer basic_inventory_slot_handle<C>::operator->() const {
	return &get_container().get<components::container>().slots[raw_id.type];
}

template <bool C>
inline typename basic_inventory_slot_handle<C>::slot_reference basic_inventory_slot_handle<C>::operator*() const {
	return *operator->();
}

template <bool C>
inline typename basic_inventory_slot_handle<C>::slot_reference basic_inventory_slot_handle<C>::get() const {
	return *operator->();
}

template <bool C>
inline bool basic_inventory_slot_handle<C>::alive() const {
	if (get_container().dead()) {
		return false;
	}

	const auto* const container = get_container().find<components::container>();

	return container && container->slots.find(raw_id.type) != container->slots.end();
}

template <bool C>
inline bool basic_inventory_slot_handle<C>::dead() const {
	return !alive();
}

template <bool C>
inline inventory_slot_id basic_inventory_slot_handle<C>::get_id() const {
	return raw_id;
}

template <bool C>
inline basic_inventory_slot_handle<C>::operator inventory_slot_id() const {
	return get_id();
}

template <bool C>
inline basic_inventory_slot_handle<C>::operator basic_inventory_slot_handle<true>() const {
	return basic_inventory_slot_handle<true>(owner, raw_id);
}