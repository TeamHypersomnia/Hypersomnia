#include "augs/ensure.h"

#include "augs/templates/container_templates.h"
#include "game/detail/inventory/inventory_utils.h"
#include "game/cosmos/cosmos.h"
#include "game/detail/entity_scripts.h"
#include "game/cosmos/entity_handle.h"
#include "game/detail/entity_handle_mixins/inventory_mixin.hpp"
#include "game/detail/inventory/inventory_slot_handle.h"
#include "game/detail/inventory/item_mounting.hpp"

#include "augs/string/string_templates.h"

void item_slot_transfer_request_params::set_specified_quantity(allocate_new_entity_access, const int quantity) {
	specified_quantity = quantity;
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

	const auto& source = item_owning_capability;
	const auto& target = target_slot_owning_capability;

	if (source.dead() && target.dead()) {
		if (target_slot_container.dead()) {
			return { capability_relation::ANONYMOUS_DROP, dead_entity };
		}

		return { capability_relation::ANONYMOUS_TRANSFER, dead_entity };
	}

	if (source.alive() && target.dead()) {
		if (target_slot_container.dead()) {
			return { capability_relation::DROP, source };
		}

		return { capability_relation::STORING_DROP, source };
	}

	if (source.alive() && target.alive()) {
		if (source != target) {
			return { capability_relation::UNMATCHING, dead_entity };
		}

		return { capability_relation::THE_SAME, source };
	}

	if (source.dead() && target.alive()) {
		return { capability_relation::PICKUP, target };
	}

	ensure(false);
	return { capability_relation::PICKUP, target };
}

item_transfer_result query_transfer_result(
	const cosmos& cosm,
	const item_slot_transfer_request r	
) {
	item_transfer_result output;
	output.result = item_transfer_result_type::INVALID_PARAMETERS;

	const auto item_handle = cosm[r.item];
	const auto target_slot = cosm[r.target_slot];

	if (item_handle.dead()) {
		return output;
	}

	item_handle.dispatch_on_having_all<components::item>(
		[&](const auto& transferred_item) {
			{
				const auto& slv = cosm.get_solvable();

				using E = entity_type_of<decltype(transferred_item)>;

				const auto cnt = slv.get_count_of<E>();
				const auto max_cnt = slv.get_max_count_of<E>();

				if (cnt + 1 >= max_cnt) {
					/* 
						Pathological case - we're running out of entity space. 
						We might need to create an entity to clone a stack.
					*/
					output.result = item_transfer_result_type::ENTITY_POOL_IS_FULL;
					return;
				}
			}

			const auto item = transferred_item.template get<components::item>();

			ensure(r.params.get_specified_quantity() != 0);

			const auto capabilities_compared = match_transfer_capabilities(transferred_item.get_cosmos(), r);
			const auto relation = capabilities_compared.relation_type;

			output.relation = relation;

			if (relation == capability_relation::UNMATCHING) {
				if (r.params.bypass_unmatching_capabilities) {

				}
				else {
					output.result = item_transfer_result_type::INVALID_CAPABILITIES;
					return;
				}
			}

			if (relation == capability_relation::DROP || relation == capability_relation::ANONYMOUS_DROP) {
				output.result = item_transfer_result_type::SUCCESSFUL_TRANSFER;

				if (r.params.get_specified_quantity() == -1) {
					output.transferred_charges = item.get_charges();
				}
				else {
					output.transferred_charges = std::min(r.params.get_specified_quantity(), item.get_charges());
				}
			}
			else {
				if (target_slot.is_child_of(transferred_item)) {
					/* Trying to insert inside the transferred item. */
					output.result = item_transfer_result_type::THE_SAME_SLOT; 
				}
				else {
					const auto containment_result = query_containment_result(
						transferred_item, 
						target_slot,
						r.params.get_specified_quantity()
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
							output.result = item_transfer_result_type::SUCCESSFUL_TRANSFER; 
							break;

						case containment_result_type::TOO_MANY_ITEMS:
							output.result = item_transfer_result_type::TOO_MANY_ITEMS;
							break;

						default: 
							output.result = item_transfer_result_type::INVALID_RESULT; 
							break;
					}
				}
			}

			if (output.is_successful() && !r.params.bypass_mounting_requirements) {
				/* Not so fast. Let's see if we can properly mount here. */
				const auto mounting_result = calc_mounting_conditions(transferred_item, target_slot);

				switch (mounting_result) {
					case mounting_conditions_type::PROGRESS:
						output.only_initiated_mounting = true;
						break;

					case mounting_conditions_type::ABORT:
						output.result = item_transfer_result_type::MOUNTING_CONDITIONS_NOT_MET;
						break;

					case mounting_conditions_type::NO_MOUNTING_REQUIRED:
						break;
				}
			}

			if (output.is_successful() ) {
				const auto t = target_slot.get_type();

				if (t == slot_function::ITEM_DEPOSIT) {
					if (capabilities_compared.authorized_capability.is_set()) {
						output.holster = true;
					}
				}
				else if (target_slot.is_hand_slot()) {
					output.wield = true;
				}
				else if (
					t == slot_function::PERSONAL_DEPOSIT
					|| t == slot_function::BELT
					|| t == slot_function::BACK
					|| t == slot_function::OVER_BACK
					|| t == slot_function::TORSO_ARMOR
					|| t == slot_function::SHOULDER
					|| t == slot_function::HAT
				) {
					output.wear = true;
				}
			}
		}
	);

	return output;
}

slot_function get_slot_with_compatible_category(const const_entity_handle item, const const_entity_handle container_entity) {
	const auto* const container = container_entity.find<invariants::container>();

	if (container) {
		if (container_entity[slot_function::ITEM_DEPOSIT].alive()) {
			return slot_function::ITEM_DEPOSIT;
		}

		for (auto&& s : container->slots) {
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
	const int specified_quantity
) {
	const auto& cosm = item_entity.get_cosmos();
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
			&& !can_stack_entities(cosm[items.at(0)], item_entity)
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
	if (a.dead() || b.dead()) {
		ensure(false && "Cannot stack dead entities!");
		return false;
	}

	return 
		a.get_flavour_id() == b.get_flavour_id() 
		&& a.get<invariants::item>().stackable
	;
}

inventory_space_type to_space_units(const std::string& s) {
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

	for (const auto& i : id.get_items_inside()) {
		charges += id.get_cosmos()[i].get<components::item>().get_charges();
	}

	return charges;
}

std::string format_space_units(const inventory_space_type u) {
	if (!u) {
		return "0";
	}

	return to_string_ex(u / double(SPACE_ATOMS_PER_UNIT), 2);
}

inventory_space_type calc_space_occupied_with_children(const const_entity_handle item_entity) {
	auto space_occupied = *item_entity.find_space_occupied();

	if (auto* const container = item_entity.find<invariants::container>()) {
		ensure_eq(item_entity.get<components::item>().get_charges(), 1);

		for (auto&& slot : container->slots) {
			if (slot.second.contributes_to_space_occupied) {
				for (const auto& entity_in_slot : item_entity[slot.first].get_items_inside()) {
					space_occupied += calc_space_occupied_with_children(item_entity.get_cosmos()[entity_in_slot]);
				}
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

