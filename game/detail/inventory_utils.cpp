#include "inventory_utils.h"
#include "game/entity_id.h"
#include "game/cosmos.h"
#include "game/components/item_component.h"

#include "templates.h"
#include "ensure.h"

entity_id get_owning_transfer_capability(entity_id entity) {
	if (entity.dead())
		return entity_id();

	auto* maybe_transfer_capability = entity->find<components::item_slot_transfers>();

	if (maybe_transfer_capability)
		return entity;

	auto* maybe_item = entity->find<components::item>();

	if (!maybe_item || maybe_item->current_slot.dead())
		return entity_id();

	return get_owning_transfer_capability(maybe_item->current_slot.container_entity);
}

inventory_slot_id first_free_hand(entity_id root_container) {
	auto maybe_primary = root_container[slot_function::PRIMARY_HAND];
	auto maybe_secondary = root_container[slot_function::SECONDARY_HAND];

	if (maybe_primary.is_empty_slot())
		return maybe_primary;

	if (maybe_secondary.is_empty_slot())
		return maybe_secondary;

	return inventory_slot_id();
}

inventory_slot_id determine_hand_holstering_slot(entity_id item_entity, entity_id searched_root_container) {
	ensure(item_entity.alive());
	ensure(searched_root_container.alive());

	auto maybe_shoulder = searched_root_container[slot_function::SHOULDER_SLOT];

	if (maybe_shoulder.alive()) {
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

		if (maybe_armor.alive())
			if (maybe_armor.can_contain(item_entity))
				return maybe_armor;
	}

	return inventory_slot_id();
}

inventory_slot_id determine_pickup_target_slot(entity_id item_entity, entity_id searched_root_container) {
	ensure(item_entity.alive());
	ensure(searched_root_container.alive());

	auto hidden_slot = determine_hand_holstering_slot(item_entity, searched_root_container);;

	if (hidden_slot.alive())
		return hidden_slot;

	if (searched_root_container[slot_function::PRIMARY_HAND].can_contain(item_entity))
		return searched_root_container[slot_function::PRIMARY_HAND];

	if (searched_root_container[slot_function::SECONDARY_HAND].can_contain(item_entity))
		return searched_root_container[slot_function::SECONDARY_HAND];

	return inventory_slot_id();
}

inventory_slot_id map_primary_action_to_secondary_hand_if_primary_empty(entity_id root_container, int is_action_secondary) {
	inventory_slot_id subject_hand;
	subject_hand.container_entity = root_container;

	auto primary = root_container[slot_function::PRIMARY_HAND];
	auto secondary = root_container[slot_function::SECONDARY_HAND];

	if (primary.is_empty_slot())
		return secondary;
	else
		return is_action_secondary ? secondary : primary;
}

item_transfer_result query_transfer_result(messages::item_slot_transfer_request r) {
	item_transfer_result output;
	auto& predicted_result = output.result;

	auto& item = r.item->get<components::item>();

	ensure(r.specified_quantity != 0);

	auto item_owning_capability = get_owning_transfer_capability(r.item);
	auto target_slot_owning_capability = get_owning_transfer_capability(r.target_slot.container_entity);

	//ensure(item_owning_capability.alive() || target_slot_owning_capability.alive());

	if (item_owning_capability.alive() && target_slot_owning_capability.alive() &&
		item_owning_capability != target_slot_owning_capability)
		predicted_result = item_transfer_result_type::INVALID_SLOT_OR_UNOWNED_ROOT;
	else if (r.target_slot.alive())
		output = containment_result(r);
	else {
		output.result = item_transfer_result_type::SUCCESSFUL_TRANSFER;
		output.transferred_charges = r.specified_quantity == -1 ? item.charges : std::min(r.specified_quantity, item.charges);
	}

	if (predicted_result == item_transfer_result_type::SUCCESSFUL_TRANSFER) {
		if (item.current_mounting == components::item::MOUNTED && !r.force_immediate_mount)
			predicted_result = item_transfer_result_type::UNMOUNT_BEFOREHAND;
	}

	return output;
}

slot_function detect_compatible_slot(entity_id item, entity_id container_entity) {
	auto* container = container_entity->find<components::container>();

	if (container) {
		if (container_entity[slot_function::ITEM_DEPOSIT].alive())
			return slot_function::ITEM_DEPOSIT;

		for (auto& s : container->slots)
			if (s.second.is_category_compatible_with(item))
				return s.first;
	}

	return slot_function::INVALID;
}

item_transfer_result containment_result(messages::item_slot_transfer_request r, bool allow_replacement) {
	item_transfer_result output;
	output.transferred_charges = 0;
	output.result = item_transfer_result_type::NO_SLOT_AVAILABLE;

	auto& item = r.item->get<components::item>();
	auto& slot = *r.target_slot;
	auto& result = output.result;

	if (item.current_slot == r.target_slot)
		result = item_transfer_result_type::THE_SAME_SLOT;
	else if (slot.always_allow_exactly_one_item && slot.items_inside.size() == 1 && !can_merge_entities(slot.items_inside[0], r.item)) {
		//if (allow_replacement) {
		//
		//}
		//auto& item_to_replace = slot.items_inside[0];
		//
		//auto& current_slot_of_replacer = item.current_slot;
		//auto replace_request = r;
		//
		//replace_request.item = slot.items_inside[0];
		//replace_request.target_slot = current_slot_of_replacer;
		//
		//if (containment_result.)
		//
		//	if (current_slot_of_replacer.alive())

		result = item_transfer_result_type::NO_SLOT_AVAILABLE;
	}
	else if (!slot.is_category_compatible_with(r.item))
		result = item_transfer_result_type::INCOMPATIBLE_CATEGORIES;
	else {
		auto space_available = r.target_slot.calculate_free_space_with_parent_containers();

		if (space_available > 0) {
			bool item_indivisible = item.charges == 1 || !item.stackable;

			if (item_indivisible) {
				if (space_available >= calculate_space_occupied_with_children(r.item)) {
					output.transferred_charges = 1;
				}
			}
			else {
				int maximum_charges_fitting_inside = space_available / item.space_occupied_per_charge;
				output.transferred_charges = std::min(item.charges, maximum_charges_fitting_inside);

				if (r.specified_quantity > -1)
					output.transferred_charges = std::min(output.transferred_charges, unsigned(r.specified_quantity));
			}
		}

		if (output.transferred_charges == 0)
			output.result = item_transfer_result_type::INSUFFICIENT_SPACE;
		else
			output.result = item_transfer_result_type::SUCCESSFUL_TRANSFER;
	}
		
	return output;
}


void for_each_descendant(entity_id item, std::function<void(entity_id item)> f) {
	f(item);

	if (item->find<components::container>()) {
		for (auto& s : item->get<components::container>().slots) {
			item[s.first].for_each_descendant(f);
		}
	}
}

#include "game/components/damage_component.h"

bool can_merge_entities(entity_id a, entity_id b) {
	return 
		components::item::can_merge_entities(a, b) &&
		components::damage::can_merge_entities(a, b);
}

unsigned to_space_units(std::string s) {
	unsigned sum = 0;
	unsigned mult = SPACE_ATOMS_PER_UNIT;

	if (s.find(".") == std::string::npos) {
		int l = s.length() - 1;
		
		while(l--)
			mult *= 10;
	}
	else {
		int l = s.find(".") - 1;

		while (l--)
			mult *= 10;
	}

	for (auto& c : s) {
		ensure(mult > 0);
		if (c == '.')
			continue;

		sum += (c - '0') * mult;
		mult /= 10;
	}

	return sum;
}

int count_charges_in_deposit(entity_id item) {
	return count_charges_inside(item[slot_function::ITEM_DEPOSIT]);
}

int count_charges_inside(inventory_slot_id id) {
	int charges = 0;

	for (auto& i : id->items_inside) {
		charges += i->get<components::item>().charges;
	}

	return charges;
}

std::wstring format_space_units(unsigned u) {
	if (!u)
		return L"0";

	return to_wstring(u / long double(SPACE_ATOMS_PER_UNIT), 2);
}

void drop_from_all_slots(entity_id c) {
	auto& container = c->get<components::container>();

	messages::item_slot_transfer_request request;

	for (auto& s : container.slots) {
		auto items_uninvalidated = s.second.items_inside;

		for (auto& i : items_uninvalidated) {
			request.item = i;
			c->get_owner_world().post_message(request);
		}
	}
}