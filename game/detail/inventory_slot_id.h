#pragma once
#include <functional>

#include "../globals/inventory.h"
#include "../components/transform_component.h"
#include "entity_system/entity_id.h"
void for_each_descendant(augs::entity_id, std::function<void(augs::entity_id item)>);

struct inventory_slot;
struct inventory_slot_id {
	slot_function type = slot_function::INVALID;
	augs::entity_id container_entity;

	bool alive() const;
	bool dead() const;

	void unset();

	void add_item(augs::entity_id);
	void remove_item(augs::entity_id);

	unsigned calculate_free_space_with_parent_containers() const;

	void for_each_descendant(std::function<void(augs::entity_id item)>);

	bool can_contain(augs::entity_id) const;

	augs::entity_id try_get_item() const;
	bool has_items() const;
	bool is_empty_slot() const;

	bool is_hand_slot() const;
	bool is_input_enabling_slot() const;

	bool should_item_inside_keep_physical_body(augs::entity_id until_parent = augs::entity_id()) const;
	float calculate_density_multiplier_due_to_being_attached() const;

	components::transform sum_attachment_offsets_of_parents(augs::entity_id attached_item) const;

	augs::entity_id get_root_container() const;

	bool operator<(const inventory_slot_id& b) const;

	inventory_slot& operator*();
	inventory_slot* operator->();
	const inventory_slot& operator*() const;
	const inventory_slot* operator->() const;
	bool operator==(inventory_slot_id b) const;
	bool operator!=(inventory_slot_id b) const;
};
