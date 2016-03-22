#include "inventory_slot.h"
#include "../components/item_component.h"
#include "../components/physics_definition_component.h"
#include "entity_system/world.h"
#include "inventory_utils.h"

inventory_slot& inventory_slot_id::operator*() {
	return container_entity->get<components::container>().slots[type];
}

bool inventory_slot_id::operator==(inventory_slot_id b) const {
	return type == b.type && container_entity == b.container_entity;
}

bool inventory_slot_id::operator<(const inventory_slot_id& b) const {
	if (container_entity == b.container_entity)
		return type < b.type;

	return container_entity < b.container_entity;
}

bool inventory_slot_id::operator!=(inventory_slot_id b) const {
	return !(*this == b);
}

inventory_slot* inventory_slot_id::operator->() {
	return &container_entity->get<components::container>().slots[type];
}

bool inventory_slot_id::alive() {
	if (container_entity.dead())
		return false;

	auto* container = container_entity->find<components::container>();

	return container && container->slots.find(type) != container->slots.end();
}

bool inventory_slot_id::dead() {
	return !alive();
}

void inventory_slot_id::unset() {
	container_entity.unset();
	type = slot_function::INVALID;
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

bool inventory_slot_id::should_item_inside_keep_physical_body(augs::entity_id until_parent) {
	bool should_item_here_keep_physical_body = (*this)->is_physical_attachment_slot;

	if (container_entity == until_parent) {
		return should_item_here_keep_physical_body;
	}

	auto* maybe_item = container_entity->find<components::item>();

	if (maybe_item) {
		//if (maybe_item->current_slot.container_entity.alive() && maybe_item->current_slot.container_entity == until_parent)
		//	return should_item_here_keep_physical_body;
		//else 
			if (maybe_item->current_slot.alive())
			return std::min(should_item_here_keep_physical_body, maybe_item->current_slot.should_item_inside_keep_physical_body(until_parent));
	}

	return should_item_here_keep_physical_body;
}

float inventory_slot_id::calculate_density_multiplier_due_to_being_attached() {
	assert((*this)->is_physical_attachment_slot);
	float density_multiplier = (*this)->attachment_density_multiplier;

	auto* maybe_item = container_entity->find<components::item>();

	if (maybe_item && maybe_item->current_slot.alive())
		return density_multiplier * maybe_item->current_slot.calculate_density_multiplier_due_to_being_attached();

	return density_multiplier;
}

components::transform inventory_slot_id::sum_attachment_offsets_of_parents(augs::entity_id attached_item) {
	auto offset = (*this)->attachment_offset;
	
	auto sticking = (*this)->attachment_sticking_mode;

	offset.pos += attached_item->get<components::physics_definition>().get_aabb_size().get_sticking_offset(sticking);
	offset.pos += container_entity->get<components::physics_definition>().get_aabb_size().get_sticking_offset(sticking);

	offset += attached_item->get<components::item>().attachment_offsets_per_sticking_mode[sticking];

	auto* maybe_item = container_entity->find<components::item>();

	if (maybe_item && maybe_item->current_slot.alive())
		return offset + maybe_item->current_slot.sum_attachment_offsets_of_parents(container_entity);

	return offset;
}

augs::entity_id inventory_slot_id::get_root_container() {
	auto* maybe_item = container_entity->find<components::item>();

	if (maybe_item && maybe_item->current_slot.alive())
		return maybe_item->current_slot.get_root_container();

	return container_entity;
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

	if (item->find<components::container>()) {
		assert(item->get<components::item>().charges == 1);
		
		for (auto& slot : item->get<components::container>().slots)
			for (auto& entity_in_slot : slot.second.items_inside)
				space_occupied += calculate_space_occupied_with_children(entity_in_slot);
	}

	return space_occupied;
}

std::vector<augs::entity_id> inventory_slot::get_mounted_items() {
	static thread_local std::vector<augs::entity_id> output;
	output.clear();
	// TODO: actually implement mounted items
	return items_inside;

	for (auto& i : items_inside) 
		if (i->get<components::item>().is_mounted()) 
			output.push_back(i);

	return output;
}

bool inventory_slot::has_unlimited_space() {
	return is_physical_attachment_slot;// || always_allow_exactly_one_item;
}

float inventory_slot::calculate_free_space_with_children() {
	if (has_unlimited_space())
		return 1000000.f;

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

bool inventory_slot::is_category_compatible_with(augs::entity_id id) {
	auto& item = id->get<components::item>();
	
	if (for_categorized_items_only && (category_allowed & item.categories_for_slot_compatibility) == 0)
		return false;
	
	return true;
}

void inventory_slot_id::for_each_descendant(std::function<void(augs::entity_id item)> f) {
	for (auto& i : (*this)->items_inside) {
		f(i);
		
		auto* container = i->find<components::container>();
		
		if (container)
			for (auto& s : container->slots) 
				i[s.first].for_each_descendant(f);
	}
}

bool inventory_slot_id::can_contain(augs::entity_id id) {
	if (dead())
		return false;

	messages::item_slot_transfer_request r;
	r.target_slot = *this;
	r.item = id;

	return containment_result(r).transferred_charges > 0;
}