#include "inventory_utils.h"
#include "game/entity_id.h"
#include "game/cosmos.h"
#include "game/components/item_component.h"
#include "game/components/damage_component.h"
#include "game/components/fixtures_component.h"
#include "game/components/physics_component.h"
#include "game/components/item_slot_transfers_component.h"
#include "game/detail/entity_scripts.h"
#include "game/messages/queue_destruction.h"
#include "game/entity_handle.h"

#include "templates.h"
#include "ensure.h"

entity_id get_owning_transfer_capability(const_entity_handle entity) {
	auto& cosmos = entity.get_cosmos();

	if (entity.dead())
		return entity_id();

	auto* maybe_transfer_capability = entity.find<components::item_slot_transfers>();

	if (maybe_transfer_capability)
		return entity;

	auto* maybe_item = entity.find<components::item>();

	if (!maybe_item || cosmos[maybe_item->current_slot].dead())
		return entity_id();

	return cosmos[maybe_item->current_slot].get_container().get_owning_transfer_capability();
}

inventory_slot_id first_free_hand(const_entity_handle root_container) {
	auto maybe_primary = root_container[slot_function::PRIMARY_HAND];
	auto maybe_secondary = root_container[slot_function::SECONDARY_HAND];

	if (maybe_primary.is_empty_slot())
		return maybe_primary;

	if (maybe_secondary.is_empty_slot())
		return maybe_secondary;

	return inventory_slot_id();
}

inventory_slot_id determine_hand_holstering_slot(const_entity_handle item_entity, const_entity_handle searched_root_container) {
	ensure(item_entity.alive());
	ensure(searched_root_container.alive());
	auto& cosmos = item_entity.get_cosmos();

	auto maybe_shoulder = searched_root_container[slot_function::SHOULDER_SLOT];

	if (maybe_shoulder.alive()) {
		if (maybe_shoulder.can_contain(item_entity))
			return maybe_shoulder;
		else if (maybe_shoulder->items_inside.size() > 0) {
			auto maybe_item_deposit = cosmos[maybe_shoulder.get_items_inside()[0]][slot_function::ITEM_DEPOSIT];

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

inventory_slot_id determine_pickup_target_slot(const_entity_handle item_entity, const_entity_handle searched_root_container) {
	ensure(item_entity.alive());
	ensure(searched_root_container.alive());
	auto& cosmos = item_entity.get_cosmos();

	auto hidden_slot = determine_hand_holstering_slot(item_entity, searched_root_container);;

	if (cosmos[hidden_slot].alive())
		return hidden_slot;

	if (searched_root_container[slot_function::PRIMARY_HAND].can_contain(item_entity))
		return searched_root_container[slot_function::PRIMARY_HAND];

	if (searched_root_container[slot_function::SECONDARY_HAND].can_contain(item_entity))
		return searched_root_container[slot_function::SECONDARY_HAND];

	return inventory_slot_id();
}

inventory_slot_id map_primary_action_to_secondary_hand_if_primary_empty(const_entity_handle root_container, int is_action_secondary) {
	inventory_slot_id subject_hand;
	subject_hand.container_entity = root_container;

	auto primary = root_container[slot_function::PRIMARY_HAND];
	auto secondary = root_container[slot_function::SECONDARY_HAND];

	if (primary.is_empty_slot())
		return secondary;
	else
		return is_action_secondary ? secondary : primary;
}

item_transfer_result query_transfer_result(const_item_slot_transfer_request r) {
	item_transfer_result output;
	auto& predicted_result = output.result;
	auto& item = r.item.get<components::item>();
	auto& cosmos = r.item.get_cosmos();

	ensure(r.specified_quantity != 0);

	auto item_owning_capability = r.item.get_owning_transfer_capability();
	auto target_slot_owning_capability = r.target_slot.get_container().get_owning_transfer_capability();

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

slot_function detect_compatible_slot(const_entity_handle item, const_entity_handle container_entity) {
	auto* container = container_entity.find<components::container>();

	if (container) {
		if (container_entity[slot_function::ITEM_DEPOSIT].alive())
			return slot_function::ITEM_DEPOSIT;

		for (auto& s : container->slots)
			if (s.second.is_category_compatible_with(item))
				return s.first;
	}

	return slot_function::INVALID;
}

item_transfer_result containment_result(const_item_slot_transfer_request r, bool allow_replacement) {
	item_transfer_result output;
	output.transferred_charges = 0;
	output.result = item_transfer_result_type::NO_SLOT_AVAILABLE;

	auto& item = r.item.get<components::item>();
	auto& cosmos = r.item.get_cosmos();
	auto& slot = *r.target_slot;
	auto& result = output.result;

	if (item.current_slot == r.target_slot)
		result = item_transfer_result_type::THE_SAME_SLOT;
	else if (slot.always_allow_exactly_one_item && slot.items_inside.size() == 1 && !can_merge_entities(cosmos[slot.items_inside[0]], r.item)) {
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

bool can_merge_entities(const_entity_handle a, const_entity_handle b) {
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

int count_charges_in_deposit(const_entity_handle item) {
	return count_charges_inside(item[slot_function::ITEM_DEPOSIT]);
}

int count_charges_inside(const_inventory_slot_handle id) {
	int charges = 0;

	for (auto& i : id->items_inside) {
		charges += id.get_cosmos()[i].get<components::item>().charges;
	}

	return charges;
}

std::wstring format_space_units(unsigned u) {
	if (!u)
		return L"0";

	return to_wstring(u / long double(SPACE_ATOMS_PER_UNIT), 2);
}

void drop_from_all_slots(entity_handle c, fixed_step& step) {
	auto& container = c.get<components::container>();

	for (auto& s : container.slots) {
		auto items_uninvalidated = s.second.items_inside;

		for (auto item : items_uninvalidated) {
			perform_transfer({ c.get_cosmos()[item], c.get_cosmos().dead_inventory_handle() }, step);
		}
	}
}

unsigned calculate_space_occupied_with_children(const_entity_handle item) {
	auto space_occupied = item.get<components::item>().get_space_occupied();

	if (item.find<components::container>()) {
		ensure(item.get<components::item>().charges == 1);

		for (auto& slot : item.get<components::container>().slots)
			for (auto& entity_in_slot : slot.second.items_inside)
				space_occupied += calculate_space_occupied_with_children(item.get_cosmos()[entity_in_slot]);
	}

	return space_occupied;
}

void add_item(inventory_slot_handle handle, entity_handle new_item) {
	handle->items_inside.push_back(new_item);
	new_item.get<components::item>().current_slot = handle;
}

void remove_item(inventory_slot_handle handle, entity_handle removed_item) {
	auto& v = handle->items_inside;
	v.erase(std::remove(v.begin(), v.end(), removed_item), v.end());
	removed_item.get<components::item>().current_slot.unset();
}

void perform_transfer(item_slot_transfer_request r, fixed_step& step) {
	auto& cosmos = r.item.get_cosmos();
	auto& item = r.item.get<components::item>();
	auto previous_slot_id = item.current_slot;
	auto previous_slot = cosmos[previous_slot_id];

	auto result = query_transfer_result(r);

	if (result.result == item_transfer_result_type::UNMOUNT_BEFOREHAND) {
		ensure(previous_slot.alive());

		item.request_unmount(r.target_slot);
		item.mark_parent_enclosing_containers_for_unmount();

		return;
	}
	else if (result.result == item_transfer_result_type::SUCCESSFUL_TRANSFER) {
		bool is_pickup_or_transfer = cosmos[r.target_slot].alive();
		bool is_drop_request = !is_pickup_or_transfer;

		components::transform previous_container_transform;

		entity_id target_item_to_stack_with;

		if (is_pickup_or_transfer) {
			for (auto& i : cosmos.get_handle(r.target_slot)->items_inside) {
				if (can_merge_entities(cosmos[r.item], cosmos[i])) {
					target_item_to_stack_with = i;
				}
			}
		}

		bool whole_item_grabbed = item.charges == result.transferred_charges;

		if (previous_slot.alive()) {
			previous_container_transform = previous_slot.get_container().get<components::transform>();

			if (whole_item_grabbed)
				remove_item(previous_slot, r.item);

			if (previous_slot.is_input_enabling_slot()) {
				unset_input_flags_of_orphaned_entity(r.item);
			}
		}

		if (cosmos[target_item_to_stack_with].alive()) {
			if (whole_item_grabbed)
				step.messages.post(messages::queue_destruction(r.item));
			else
				item.charges -= result.transferred_charges;

			cosmos[target_item_to_stack_with].get<components::item>().charges += result.transferred_charges;

			return;
		}

		entity_id grabbed_item_part;
		entity_id new_charge_stack;

		if (whole_item_grabbed)
			grabbed_item_part = r.item;
		else {
			new_charge_stack = cosmos.clone_entity(r.item);
			item.charges -= result.transferred_charges;
			cosmos[new_charge_stack].get<components::item>().charges = result.transferred_charges;

			grabbed_item_part = new_charge_stack;
		}

		if (is_pickup_or_transfer)
			add_item(r.target_slot, cosmos[grabbed_item_part]);

		for_each_descendant(cosmos[grabbed_item_part], [previous_container_transform, new_charge_stack](entity_handle descendant) {
			auto& cosmos = descendant.get_cosmos();

			auto parent_slot = cosmos[descendant.get<components::item>().current_slot];
			auto def = descendant.get<components::fixtures>().get_definition();

			if (parent_slot.alive()) {
				def.activated = parent_slot.should_item_inside_keep_physical_body();
				def.owner_body = parent_slot.get_root_container();
				def.offsets_for_created_shapes[int(components::fixtures::offset_type::ITEM_ATTACHMENT_DISPLACEMENT)]
					= parent_slot.sum_attachment_offsets_of_parents(descendant);
				def.offsets_for_created_shapes[int(components::fixtures::offset_type::SPECIAL_MOVE_DISPLACEMENT)].reset();
			}
			else {
				def.activated = true;
				def.owner_body = descendant;
				def.offsets_for_created_shapes[int(components::fixtures::offset_type::ITEM_ATTACHMENT_DISPLACEMENT)].reset();
				def.offsets_for_created_shapes[int(components::fixtures::offset_type::SPECIAL_MOVE_DISPLACEMENT)].reset();
			}

			descendant.get<components::fixtures>().initialize_from_definition(def);
			descendant.get<components::transform>() = previous_container_transform;
		});

		auto& grabbed_item = cosmos[grabbed_item_part].get<components::item>();

		if (is_pickup_or_transfer) {
			if (r.target_slot->items_need_mounting) {
				grabbed_item.intended_mounting = components::item::MOUNTED;

				if (r.force_immediate_mount) {
					grabbed_item.current_mounting = components::item::MOUNTED;
				}
			}
		}

		if (is_drop_request) {
			auto force = vec2().set_from_degrees(previous_container_transform.rotation).set_length(60);
			auto offset = vec2().random_on_circle(20, cosmos.get_rng_for(r.item));

			auto& physics = cosmos[grabbed_item_part].get<components::physics>();
			physics.apply_force(force, offset, true);
			physics.since_dropped.set(200);
		}
	}
}