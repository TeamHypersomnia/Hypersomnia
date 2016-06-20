#pragma once
#include <functional>

#include "inventory_slot_handle_declaration.h"
#include "game/entity_handle_declaration.h"
#include "game/entity_id.h"
#include "game/globals/slot_function.h"
#include "game/detail/inventory_slot_id.h"
#include "game/types_specification/storage_instantiation.h"

struct inventory_slot;

namespace components {
	struct transform;
}

template <bool is_const>
class basic_inventory_slot_handle {
	typedef basic_entity_handle<is_const> entity_handle_type;
	
	typedef typename std::conditional<is_const, const storage_for_all_components_and_aggregates&, storage_for_all_components_and_aggregates&>::type pool_reference;
	typedef typename std::conditional<is_const, const inventory_slot&, inventory_slot&>::type slot_reference;
	typedef typename std::conditional<is_const, const inventory_slot*, inventory_slot*>::type slot_pointer;

	entity_handle_type get_handle() const;
	entity_handle_type make_handle(entity_id) const;
public:
	basic_inventory_slot_handle(pool_reference, inventory_slot_id);
	
	pool_reference owner;
	inventory_slot_id raw_id;

	void unset();

	void add_item(entity_id) const;
	void remove_item(entity_id) const;

	void for_each_descendant(std::function<void(entity_handle_type item)>) const;

	slot_reference operator*() const;
	slot_pointer operator->() const;

	bool alive() const;
	bool dead() const;

	bool can_contain(entity_id) const;

	entity_handle_type try_get_item() const;
	entity_handle_type get_root_container() const;

	bool has_items() const;
	bool is_empty_slot() const;

	bool is_hand_slot() const;
	bool is_input_enabling_slot() const;

	float calculate_density_multiplier_due_to_being_attached() const;

	components::transform sum_attachment_offsets_of_parents(entity_id attached_item) const;

	bool should_item_inside_keep_physical_body(entity_id until_parent = entity_id()) const;

	unsigned calculate_free_space_with_parent_containers() const;

	inventory_slot_id get_id() const;
	operator inventory_slot_id() const;
};
