#pragma once
#include <functional>

#include "inventory_slot_handle_declaration.h"
#include "game/entity_handle_declaration.h"
#include "game/entity_id.h"
#include "game/enums/slot_function.h"
#include "game/detail/inventory_slot_id.h"

struct inventory_slot;

namespace components {
	struct transform;
}

class cosmos;

template <bool is_const>
class basic_inventory_slot_handle {
	typedef basic_entity_handle<is_const> entity_handle_type;
	
	typedef typename std::conditional<is_const, const cosmos&, cosmos&>::type owner_reference;
	typedef typename std::conditional<is_const, const inventory_slot&, inventory_slot&>::type slot_reference;
	typedef typename std::conditional<is_const, const inventory_slot*, inventory_slot*>::type slot_pointer;

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

	void for_each_descendant(std::function<void(entity_handle_type item)>) const;

	slot_reference operator*() const;
	slot_pointer operator->() const;

	bool alive() const;
	bool dead() const;

	bool can_contain(entity_id) const;

	entity_handle_type try_get_item() const;
	entity_handle_type get_container() const;
	entity_handle_type get_root_container() const;

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
};
