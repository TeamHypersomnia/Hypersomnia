#include "container_component.h"
#include "item_component.h"
#include "entity_system/entity_id.h"
#include "entity_system/entity.h"

namespace components {
	container::slot& container::slot_id::operator*() {
		return container_entity->get<components::container>().slots[type];
	}

	container::slot* container::slot_id::operator->()  {
		return &container_entity->get<components::container>().slots[type];
	}

	bool container::slot_id::alive() {
		return container_entity.alive() && container_entity->get<components::container>().slots.find(type) != container_entity->get<components::container>().slots.end();
	}

	bool container::slot_id::dead() {
		return !alive();
	}

	void container::slot_id::unset() {
		container_entity.unset();
	}

	bool container::slot_id::is_hand_slot() {
		return type == slot_function::PRIMARY_HAND || type == slot_function::SECONDARY_HAND;
	}

	bool container::slot_id::should_item_inside_keep_physical_body() {
		bool should_item_here_keep_physical_body = (*this)->disregard_space_and_allow_one_entity;

		auto* maybe_item = container_entity->find<components::item>();

		if (maybe_item && maybe_item->current_slot.alive())
			return std::min(should_item_here_keep_physical_body, maybe_item->current_slot.should_item_inside_keep_physical_body());

		return should_item_here_keep_physical_body;
	}

	void container::slot_id::add_item(augs::entity_id id) {
		(*this)->items_inside.push_back(id);
		id->get<components::item>().current_slot = *this;
	}

	void container::slot_id::remove_item(augs::entity_id id) {
		auto& v = (*this)->items_inside;
		v.erase(std::remove(v.begin(), v.end(), id), v.end());
		id->get<components::item>().current_slot.unset();
	}
	
	unsigned container::calculate_space_occupied_with_children(augs::entity_id item) {
		unsigned occupied = item->get<components::item>().space_occupied;

		if (item->find<components::container>())
			for (auto& slot : item->get<components::container>().slots)
				for (auto& entity_in_slot : slot.second.items_inside)
					occupied += calculate_space_occupied_with_children(entity_in_slot);

		return occupied;
	}

	unsigned container::slot::calculate_free_space_with_children() {
		if (disregard_space_and_allow_one_entity)
			return std::numeric_limits<unsigned>::max();

		unsigned space = space_available;

		for (auto& e : items_inside)
			space -= calculate_space_occupied_with_children(e);

		return space;
	}

	unsigned container::slot_id::calculate_free_space_with_parent_containers() {
		unsigned maximum_space = (*this)->calculate_free_space_with_children();

		auto* maybe_item = container_entity->find<components::item>();

		if (maybe_item && maybe_item->current_slot.alive())
			return std::min(maximum_space, maybe_item->current_slot.calculate_free_space_with_parent_containers());
		
		return maximum_space;
	}

	bool container::slot_id::can_contain(augs::entity_id id) {
		auto& item = id->get<components::item>();
		auto& slot = **this;

		if (slot.disregard_space_and_allow_one_entity && slot.items_inside.size() > 0)
			return false;

		if (slot.for_categorized_items_only && (slot.category_allowed & item.categories_for_slot_compatibility) == 0)
			return false;

		return calculate_free_space_with_parent_containers() >= item.space_occupied;
	}
}