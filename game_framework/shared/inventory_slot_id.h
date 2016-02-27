#pragma once
#include "../globals/inventory.h"
#include "entity_system/entity_id.h"

struct inventory_slot;
struct inventory_slot_id {
	slot_function type = slot_function::ITEM_DEPOSIT;
	augs::entity_id container_entity;

	bool functional();

	bool alive();
	bool dead();

	void unset();

	void add_item(augs::entity_id);
	void remove_item(augs::entity_id);

	float calculate_free_space_with_parent_containers();
	bool can_contain(augs::entity_id);

	bool has_items();
	bool is_empty_slot();

	bool is_hand_slot();
	bool should_item_inside_keep_physical_body();

	bool operator<(const inventory_slot_id& b) const;

	inventory_slot& operator*();
	inventory_slot* operator->();
	bool operator==(inventory_slot_id b) const;
	bool operator!=(inventory_slot_id b) const;
};
