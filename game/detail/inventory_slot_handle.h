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

	entity_handle_type get_handle() const;
public:
	basic_inventory_slot_handle(owner_reference, inventory_slot_id);
	
	std::vector<entity_handle_type> get_mounted_items() const;

	owner_reference get_cosmos() const;

	owner_reference owner;
	inventory_slot_id raw_id;

	void unset();
	entity_handle_type make_handle(entity_id) const;
	basic_inventory_slot_handle make_handle(inventory_slot_id) const;

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

	bool can_contain(entity_id) const;

	entity_handle_type try_get_item() const;
	entity_handle_type get_container() const;
	entity_handle_type get_root_container() const;

	std::vector<entity_handle_type> get_items_inside() const;

	bool has_items() const;
	bool is_empty_slot() const;

	bool is_hand_slot() const;
	bool is_input_enabling_slot() const;

	float calculate_density_multiplier_due_to_being_attached() const;
	unsigned calculate_free_space_with_children() const;

	components::transform sum_attachment_offsets_of_parents(entity_id attached_item) const;

	bool should_item_inside_keep_physical_body(entity_id until_parent = entity_id()) const;

	unsigned calculate_free_space_with_parent_containers() const;

	inventory_slot_id get_id() const;
	operator inventory_slot_id() const;
	operator basic_inventory_slot_handle<true>() const;

	//typedef maybe_const_ref_t<is_const, slot_button> slot_button_ref;
	//slot_button_ref get_button() const;
};
