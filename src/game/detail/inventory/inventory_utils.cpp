#include "augs/ensure.h"

#include "augs/templates/container_templates.h"
#include "game/detail/inventory/inventory_utils.h"
#include "game/transcendental/cosmos.h"
#include "game/detail/entity_scripts.h"
#include "game/transcendental/entity_handle.h"

#include "augs/string/string_templates.h"

bool capability_comparison::is_legal() const {
	return
		relation_type == capability_relation::DROP
		|| relation_type == capability_relation::PICKUP
		|| relation_type == capability_relation::THE_SAME
	;
}

bool capability_comparison::is_authorized(const entity_id h) const {
	return is_legal() && authorized_capability == h;
}

capability_comparison match_transfer_capabilities(
	const cosmos& cosm,
	item_slot_transfer_request r
) {
	const auto transferred_item = cosm[r.item];
	// const auto target_slot = cosm[r.target_slot];
	const auto target_slot_container = cosm[r.target_slot].get_container();

	const auto dead_entity = transferred_item.get_cosmos()[entity_id()];

	const auto item_owning_capability = transferred_item.get_owning_transfer_capability();
	const auto target_slot_owning_capability = target_slot_container.get_owning_transfer_capability();

	if (/* both_dead */ target_slot_owning_capability.dead() && item_owning_capability.dead()) {
		if (target_slot_container) {
			return { capability_relation::PICKUP, dead_entity };
		}

		return { capability_relation::DROP, dead_entity };
	}

	if (/* both_alive */ item_owning_capability && target_slot_owning_capability) {
		if (item_owning_capability == target_slot_owning_capability) {
			return { capability_relation::THE_SAME, item_owning_capability };
		}

		return { capability_relation::UNMATCHING, dead_entity };
	}

	if (target_slot_owning_capability.dead() && item_owning_capability.alive()) {
		return { capability_relation::DROP, item_owning_capability };
	}

	if (target_slot_owning_capability.alive() && item_owning_capability.dead()) {
		return { capability_relation::PICKUP, target_slot_owning_capability };
	}

	ensure(false);
	return { capability_relation::PICKUP, target_slot_owning_capability };
}

item_transfer_result query_transfer_result(
	const cosmos& cosm,
	const item_slot_transfer_request r	
) {
	item_transfer_result output;
	const auto transferred_item = cosm[r.item];
	const auto target_slot = cosm[r.target_slot];
	const auto item = transferred_item.get<components::item>();

	ensure(r.specified_quantity != 0);

	const auto capabilities_compared = match_transfer_capabilities(transferred_item.get_cosmos(), r);
	const auto result = capabilities_compared.relation_type;

	if (result == capability_relation::UNMATCHING) {
		output.result = item_transfer_result_type::INVALID_CAPABILITIES;
	}
	else if (result == capability_relation::DROP) {
		output.result = item_transfer_result_type::SUCCESSFUL_DROP;

		if (r.specified_quantity == -1) {
			output.transferred_charges = item.get_charges();
		}
		else {
			output.transferred_charges = std::min(r.specified_quantity, item.get_charges());
		}
	}
	else {
		ensure(capabilities_compared.is_legal());

		const bool trying_to_insert_inside_the_transferred_item = target_slot.is_child_of(transferred_item);
		ensure(!trying_to_insert_inside_the_transferred_item);

		const auto containment_result = query_containment_result(
			transferred_item, 
			target_slot,
			r.specified_quantity
		);

		output.transferred_charges = containment_result.transferred_charges;

		switch (containment_result.result) {
			case containment_result_type::INCOMPATIBLE_CATEGORIES: 
				output.result = item_transfer_result_type::INCOMPATIBLE_CATEGORIES; 
				break;

			case containment_result_type::INSUFFICIENT_SPACE: 
				output.result = item_transfer_result_type::INSUFFICIENT_SPACE; 
				break;

			case containment_result_type::THE_SAME_SLOT: 
				output.result = item_transfer_result_type::THE_SAME_SLOT; 
				break;

			case containment_result_type::SUCCESSFUL_CONTAINMENT:
				if (result == capability_relation::PICKUP) {
					output.result = item_transfer_result_type::SUCCESSFUL_PICKUP;
				}
				else {
					output.result = item_transfer_result_type::SUCCESSFUL_TRANSFER; 
				}
				break;

			case containment_result_type::TOO_MANY_ITEMS:
				output.result = item_transfer_result_type::TOO_MANY_ITEMS;
				break;

			default: 
				output.result = item_transfer_result_type::INVALID_RESULT; 
				break;
		}
	}

#if TODO_MOUNTING
	 if (predicted_result == item_transfer_result_type::SUCCESSFUL_TRANSFER) {
	 	if (item.current_mounting == components::item::MOUNTED && !r.force_immediate_mount) {
	 		predicted_result = item_transfer_result_type::UNMOUNT_BEFOREHAND;
	 	}
	 }
#endif

	return output;
}

slot_function get_slot_with_compatible_category(const const_entity_handle item, const const_entity_handle container_entity) {
	const auto* const container = container_entity.find<invariants::container>();

	if (container) {
		if (container_entity[slot_function::ITEM_DEPOSIT].alive()) {
			return slot_function::ITEM_DEPOSIT;
		}

		for (const auto& s : container->slots) {
			if (s.second.is_category_compatible_with(item)) {
				return s.first;
			}
		}
	}

	return slot_function::INVALID;
}

containment_result query_containment_result(
	const const_entity_handle item_entity,
	const const_inventory_slot_handle target_slot,
	const int specified_quantity,
	const bool /* allow_replacement */
) {
	const auto& cosmos = item_entity.get_cosmos();
	const auto item = item_entity.get<components::item>();
	const auto& item_def = item_entity.get<invariants::item>();
	const auto& slot = *target_slot;

	containment_result output;
	auto& result = output.result;

	if (item.get_current_slot() == target_slot) {
		result = containment_result_type::THE_SAME_SLOT;
	}
	else if (!slot.is_category_compatible_with(item_entity)) {
		result = containment_result_type::INCOMPATIBLE_CATEGORIES;
	}
	else {
		const auto& items = target_slot.get_items_inside();

		/* TODO: If we want to do so, impose a limit on the number of items in a container here. */

		const bool slot_would_have_too_many_items =
			slot.always_allow_exactly_one_item
			&& items.size() == 1
			&& !can_stack_entities(cosmos[target_slot.get_items_inside().at(0)], item_entity)
		;

		if (slot_would_have_too_many_items) {
			result = containment_result_type::TOO_MANY_ITEMS;
		}
		else {
			const auto rsa = target_slot.calc_real_space_available();

			if (rsa > 0) {
				const bool item_indivisible = item.get_charges() == 1 || !item_def.stackable;

				if (item_indivisible) {
					if (rsa >= calc_space_occupied_with_children(item_entity)) {
						output.transferred_charges = 1;
					}
				}
				else {
					const int maximum_charges_fitting_inside = rsa / item_def.space_occupied_per_charge;
					output.transferred_charges = std::min(item.get_charges(), maximum_charges_fitting_inside);

					if (specified_quantity > -1) {
						output.transferred_charges = std::min(output.transferred_charges, static_cast<unsigned>(specified_quantity));
					}
				}
			}

			if (output.transferred_charges == 0) {
				output.result = containment_result_type::INSUFFICIENT_SPACE;
			}
			else {
				output.result = containment_result_type::SUCCESSFUL_CONTAINMENT;
			}
		}
	}
		
	return output;
}

bool can_stack_entities(
	const const_entity_handle a,
	const const_entity_handle b
) {
	const auto name = a.get_name();
	return name == b.get_name() && a.get<invariants::item>().stackable;
}

unsigned to_space_units(const std::string& s) {
	unsigned sum = 0;
	unsigned mult = SPACE_ATOMS_PER_UNIT;

	if (s.find('.') == std::string::npos) {
		auto l = static_cast<int>(s.length()) - 1;
		
		while (l--) {
			mult *= 10;
		}
	}
	else {
		int l = static_cast<int>(s.find('.')) - 1;

		while (l--) {
			mult *= 10;
		}
	}

	for (auto& c : s) {
		ensure(mult > 0);
		
		if (c == '.') {
			continue;
		}

		sum += (c - '0') * mult;
		mult /= 10;
	}

	return sum;
}

int count_charges_in_deposit(const const_entity_handle item) {
	return count_charges_inside(item[slot_function::ITEM_DEPOSIT]);
}

int count_charges_inside(const const_inventory_slot_handle id) {
	int charges = 0;

	for (const auto i : id.get_items_inside()) {
		charges += id.get_cosmos()[i].get<components::item>().get_charges();
	}

	return charges;
}

std::string format_space_units(const unsigned u) {
	if (!u) {
		return "0";
	}

	return to_string_ex(u / double(SPACE_ATOMS_PER_UNIT), 2);
}

unsigned calc_space_occupied_with_children(const const_entity_handle item_entity) {
	auto space_occupied = *item_entity.find_space_occupied();

	if (auto* const container = item_entity.find<invariants::container>()) {
		ensure_eq(item_entity.get<components::item>().get_charges(), 1);

		for (const auto& slot : container->slots) {
			for (const auto entity_in_slot : get_items_inside(item_entity, slot.first)) {
				space_occupied += calc_space_occupied_with_children(item_entity.get_cosmos()[entity_in_slot]);
			}
		}
	}

	return space_occupied;
}

augs::constant_size_vector<item_slot_transfer_request, 4> swap_slots_for_items(
	const const_entity_handle first_handle,
	const const_entity_handle second_handle
) {
	augs::constant_size_vector<item_slot_transfer_request, 4> output;

	const auto first_slot = first_handle.get_current_slot();
	const auto second_slot = second_handle.get_current_slot();

	ensure(first_handle.alive());
	ensure(second_handle.alive());

	output.push_back(item_slot_transfer_request::drop(first_handle));
	output.push_back(item_slot_transfer_request::drop(second_handle));

	output.push_back(item_slot_transfer_request::standard(first_handle, second_slot));
	output.push_back(item_slot_transfer_request::standard(second_handle, first_slot));

	return output;
}

