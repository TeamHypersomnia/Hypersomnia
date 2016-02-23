#include "inventory_slot.h"
#include "../components/item_component.h"
#include "entity_system/world.h"

inventory_slot& inventory_slot_id::operator*() {
	return container_entity->get<components::container>().slots[type];
}

bool inventory_slot_id::operator==(inventory_slot_id b) const {
	return b.type == type && b.container_entity == b.container_entity;
}

bool inventory_slot_id::operator!=(inventory_slot_id b) const {
	return !(*this == b);
}

inventory_slot* inventory_slot_id::operator->() {
	return &container_entity->get<components::container>().slots[type];
}

bool inventory_slot_id::alive() {
	return container_entity.alive() && container_entity->get<components::container>().slots.find(type) != container_entity->get<components::container>().slots.end();
}

bool inventory_slot_id::functional() {
	return alive();
}

bool inventory_slot_id::dead() {
	return !alive();
}

void inventory_slot_id::unset() {
	container_entity.unset();
}

bool inventory_slot_id::is_hand_slot() {
	return type == slot_function::PRIMARY_HAND || type == slot_function::SECONDARY_HAND;
}

bool inventory_slot_id::has_items() {
	return alive() && (*this)->items_inside.size() > 0;
}

bool inventory_slot_id::is_empty_slot() {
	return alive() && (*this)->items_inside.size() == 0;
}

bool inventory_slot_id::should_item_inside_keep_physical_body() {
	bool should_item_here_keep_physical_body = (*this)->is_attachment_slot;

	auto* maybe_item = container_entity->find<components::item>();

	if (maybe_item && maybe_item->current_slot.alive())
		return std::min(should_item_here_keep_physical_body, maybe_item->current_slot.should_item_inside_keep_physical_body());

	return should_item_here_keep_physical_body;
}

void inventory_slot_id::add_item(augs::entity_id id) {
	(*this)->items_inside.push_back(id);
	id->get<components::item>().current_slot = *this;
}

void inventory_slot_id::remove_item(augs::entity_id id) {
	auto& v = (*this)->items_inside;
	v.erase(std::remove(v.begin(), v.end(), id), v.end());
	id->get<components::item>().current_slot.unset();
}

float calculate_space_occupied_with_children(augs::entity_id item) {
	auto space_occupied = item->get<components::item>().get_space_occupied();

	if (item->find<components::container>())
		for (auto& slot : item->get<components::container>().slots)
			for (auto& entity_in_slot : slot.second.items_inside)
				space_occupied += calculate_space_occupied_with_children(entity_in_slot);

	return space_occupied;
}

std::vector<augs::entity_id> inventory_slot::get_mounted_items() {
	static thread_local std::vector<augs::entity_id> output;
	output.clear();

	for (auto& i : items_inside) 
		if (i->get<components::item>().is_mounted()) 
			output.push_back(i);

	return output;
}

float inventory_slot::calculate_free_space_with_children() {
	if (is_attachment_slot)
		return std::numeric_limits<unsigned>::max();

	float space = space_available;

	for (auto& e : items_inside)
		space -= calculate_space_occupied_with_children(e);

	return space;
}

float inventory_slot_id::calculate_free_space_with_parent_containers() {
	auto maximum_space = (*this)->calculate_free_space_with_children();

	auto* maybe_item = container_entity->find<components::item>();

	if (maybe_item && maybe_item->current_slot.alive())
		return std::min(maximum_space, maybe_item->current_slot.calculate_free_space_with_parent_containers());

	return maximum_space;
}

bool inventory_slot_id::can_contain(augs::entity_id id) {
	if (dead())
		return false;

	auto& item = id->get<components::item>();
	auto& slot = **this;

	if (item.current_slot == *this)
		return false;

	if (slot.is_attachment_slot && slot.items_inside.size() > 0)
		return false;

	if (slot.for_categorized_items_only && (slot.category_allowed & item.categories_for_slot_compatibility) == 0)
		return false;

	return calculate_free_space_with_parent_containers() >= item.get_space_occupied();
}

augs::entity_id get_root_container(augs::entity_id entity) {
	auto* maybe_item = entity->find<components::item>();

	if (!maybe_item || maybe_item->current_slot.dead())
		return entity;

	else {
		return get_root_container(maybe_item->current_slot.container_entity);
	}
}

inventory_slot_id first_free_hand(augs::entity_id root_container) {
	auto maybe_primary = root_container[slot_function::PRIMARY_HAND];
	auto maybe_secondary = root_container[slot_function::SECONDARY_HAND];

	if (maybe_primary.is_empty_slot())
		return maybe_primary;

	if (maybe_secondary.is_empty_slot())
		return maybe_secondary;

	return inventory_slot_id();
}

inventory_slot_id determine_hand_holstering_slot(augs::entity_id item_entity, augs::entity_id searched_root_container) {
	assert(item_entity.alive());
	assert(searched_root_container.alive());

	auto maybe_shoulder = searched_root_container[slot_function::SHOULDER_SLOT];

	if (maybe_shoulder.alive() && maybe_shoulder->is_holsterable()) {
		if (maybe_shoulder.can_contain(item_entity))
			return maybe_shoulder;
		else if (maybe_shoulder->items_inside.size() > 0) {
			auto maybe_item_deposit = maybe_shoulder->items_inside[0][slot_function::ITEM_DEPOSIT];

			if (maybe_item_deposit.alive() && maybe_item_deposit.can_contain(item_entity))
				return maybe_item_deposit;
		}
	}
	else {
		auto maybe_armor = searched_root_container[slot_function::TORSO_ARMOR_SLOT];
		
		if (maybe_armor.alive() && maybe_armor->is_holsterable())
			if (maybe_armor.can_contain(item_entity))
				return maybe_armor;
	}

	return inventory_slot_id();
}

inventory_slot_id determine_pickup_target_slot(augs::entity_id item_entity, augs::entity_id searched_root_container) {
	assert(item_entity.alive());
	assert(searched_root_container.alive());

	auto hidden_slot = determine_hand_holstering_slot(item_entity, searched_root_container);;

	if (hidden_slot.alive())
		return hidden_slot;

	auto hand = first_free_hand(searched_root_container);
	
	if (hand.can_contain(item_entity))
		return hand;

	return inventory_slot_id();
}

inventory_slot_id map_primary_action_to_secondary_hand_if_primary_empty(augs::entity_id root_container, int is_action_secondary) {
	inventory_slot_id subject_hand;
	subject_hand.container_entity = root_container;

	auto primary = root_container[slot_function::PRIMARY_HAND];
	auto secondary = root_container[slot_function::SECONDARY_HAND];

	if (primary.is_empty_slot())
		return secondary;
	else
		return is_action_secondary ? secondary : primary;
}