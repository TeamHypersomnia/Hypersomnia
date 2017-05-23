#include "inventory_utils.h"
#include "game/transcendental/entity_id.h"
#include "game/transcendental/cosmos.h"
#include "game/components/item_component.h"
#include "game/components/missile_component.h"
#include "game/components/fixtures_component.h"
#include "game/components/rigid_body_component.h"
#include "game/components/special_physics_component.h"
#include "game/components/item_slot_transfers_component.h"
#include "game/components/name_component.h"
#include "game/components/motor_joint_component.h"
#include "game/detail/gui/character_gui.h"
#include "game/detail/entity_scripts.h"
#include "game/messages/queue_destruction.h"
#include "game/messages/item_picked_up_message.h"
#include "game/messages/interpolation_correction_request.h"
#include "game/transcendental/entity_handle.h"

#include "augs/ensure.h"
#include "game/transcendental/logic_step.h"
#include "augs/templates/string_templates.h"
#include "game/detail/gui/gui_positioning.h"


#include "game/transcendental/data_living_one_step.h"

bool capability_comparison::is_legal() const {
	return
		relation_type == capability_relation::DROP
		|| relation_type == capability_relation::PICKUP
		|| relation_type == capability_relation::THE_SAME
		|| relation_type == capability_relation::BOTH_DEAD
	;
}

bool capability_comparison::is_authorized(const const_entity_handle h) const {
	return is_legal() && authorized_capability == h;
}

capability_comparison match_transfer_capabilities(
	const cosmos& cosm,
	item_slot_transfer_request r
) {
	const auto transferred_item = cosm[r.item];
	// const auto target_slot = cosm[r.target_slot];
	const auto target_slot_container = cosm[r.target_slot].get_container();

	const auto& dead_entity = transferred_item.get_cosmos()[entity_id()];

	const auto item_owning_capability = transferred_item.get_owning_transfer_capability();
	const auto target_slot_owning_capability = target_slot_container.get_owning_transfer_capability();

	if (target_slot_owning_capability.dead() && item_owning_capability.dead()) {
		return { capability_relation::BOTH_DEAD, dead_entity };
	}

	if (
		item_owning_capability.alive()
		&& target_slot_owning_capability.alive()
		&& item_owning_capability != target_slot_owning_capability
	) {
		return { capability_relation::UNMATCHING, dead_entity };
	}

	if (
		item_owning_capability.alive()
		&& target_slot_owning_capability.alive()
		&& item_owning_capability == target_slot_owning_capability
	) {
		return { capability_relation::THE_SAME, item_owning_capability };
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
	const auto& item = transferred_item.get<components::item>();

	ensure(r.specified_quantity != 0);

	const auto capabilities_compared = match_transfer_capabilities(transferred_item.get_cosmos(), r);
	const auto result = capabilities_compared.relation_type;

	if (result == capability_relation::UNMATCHING) {
		output.result = item_transfer_result_type::INVALID_CAPABILITIES;
	}
	else if (result == capability_relation::DROP) {
		output.result = item_transfer_result_type::SUCCESSFUL_DROP;

		if (r.specified_quantity == -1) {
			output.transferred_charges = item.charges;
		}
		else {
			output.transferred_charges = std::min(r.specified_quantity, item.charges);
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

	// if (predicted_result == item_transfer_result_type::SUCCESSFUL_TRANSFER) {
	// 	if (item.current_mounting == components::item::MOUNTED && !r.force_immediate_mount) {
	// 		predicted_result = item_transfer_result_type::UNMOUNT_BEFOREHAND;
	// 	}
	// }

	return output;
}

slot_function get_slot_with_compatible_category(const const_entity_handle item, const const_entity_handle container_entity) {
	const auto* const container = container_entity.find<components::container>();

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
	int specified_quantity,
	bool allow_replacement
) {
	const auto& cosmos = item_entity.get_cosmos();
	const auto& item = item_entity.get<components::item>();
	const auto& slot = *target_slot;

	containment_result output;
	auto& result = output.result;

	if (item.current_slot == target_slot) {
		result = containment_result_type::THE_SAME_SLOT;
	}
	else if (!slot.is_category_compatible_with(item_entity)) {
		result = containment_result_type::INCOMPATIBLE_CATEGORIES;
	}
	else {
		const bool slot_would_have_too_many_items =
			slot.items_inside.size() == slot.items_inside.capacity()
			|| (
				slot.always_allow_exactly_one_item
				&& slot.items_inside.size() == 1
				&& !can_stack_entities(cosmos[target_slot.get_items_inside().at(0)], item_entity)
			)
		;

		if (slot_would_have_too_many_items) {
			result = containment_result_type::TOO_MANY_ITEMS;
		}
		else {
			const auto rsa = target_slot.calculate_real_space_available();

			if (rsa > 0) {
				const bool item_indivisible = item.charges == 1 || !item.stackable;

				if (item_indivisible) {
					if (rsa >= calculate_space_occupied_with_children(item_entity)) {
						output.transferred_charges = 1;
					}
				}
				else {
					const int maximum_charges_fitting_inside = rsa / item.space_occupied_per_charge;
					output.transferred_charges = std::min(item.charges, maximum_charges_fitting_inside);

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
	const auto& cosmos = a.get_cosmos();
	
	const auto name = a.get_name();

	if (name == b.get_name()) 
		if(
			name == L"Red charge"
			|| name == L"Cyan charge"
			|| name == L"Pink charge"
			|| name == L"Green charge"
		) {
			return true;
		}
	}

	//const auto catridge_a = a.find<components::missile>();
	//const auto catridge_b = b.find<components::missile>();
	//
	//const auto missile_a = a.find<components::missile>();
	//const auto missile_b = b.find<components::missile>();
	//
	//if (missile_a != nullptr && missile_b != nullptr) {
	//	if (trivial_compare(*missile_a, *missile_b)) {
	//
	//	}
	//}

	return false;
}

unsigned to_space_units(const std::string& s) {
	unsigned sum = 0;
	unsigned mult = SPACE_ATOMS_PER_UNIT;

	if (s.find(".") == std::string::npos) {
		int l = s.length() - 1;
		
		while (l--) {
			mult *= 10;
		}
	}
	else {
		int l = s.find(".") - 1;

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

	for (const auto i : id->items_inside) {
		charges += id.get_cosmos()[i].get<components::item>().charges;
	}

	return charges;
}

std::wstring format_space_units(const unsigned u) {
	if (!u) {
		return L"0";
	}

	return to_wstring(u / long double(SPACE_ATOMS_PER_UNIT), 2);
}

void drop_from_all_slots(const entity_handle c, const logic_step step) {
	const auto& container = c.get<components::container>();

	for (const auto& s : container.slots) {
		for (const auto item : s.second.items_inside) {
			perform_transfer( item_slot_transfer_request{ item, inventory_slot_id() }, step);
		}
	}
}

unsigned calculate_space_occupied_with_children(const const_entity_handle item) {
	auto space_occupied = item.get<components::item>().get_space_occupied();

	if (item.find<components::container>()) {
		ensure(item.get<components::item>().charges == 1);

		for (const auto& slot : item.get<components::container>().slots) {
			for (const auto entity_in_slot : slot.second.items_inside) {
				space_occupied += calculate_space_occupied_with_children(item.get_cosmos()[entity_in_slot]);
			}
		}
	}

	return space_occupied;
}

void detail_add_item(const inventory_slot_handle handle, const entity_handle new_item) {
	handle->items_inside.push_back(new_item);
	new_item.get<components::item>().current_slot = handle;
}

void detail_remove_item(const inventory_slot_handle handle, const entity_handle removed_item) {
	auto& v = handle->items_inside;
	erase_element(v, removed_item);
	removed_item.get<components::item>().current_slot.unset();
}

components::transform sum_attachment_offsets(const const_logic_step step, const inventory_item_address addr) {
	const auto& cosm = step.cosm;
	components::transform total;

	inventory_slot_id current_slot;
	current_slot.container_entity = addr.root_container;
	
	for (size_t i = 0; i < addr.directions.size(); ++i) {
		current_slot.type = addr.directions[i];

		const auto slot_handle = cosm[current_slot];

		ensure(slot_handle->makes_physical_connection());
		ensure(slot_handle->always_allow_exactly_one_item);

		const auto item_in_slot = slot_handle.get_items_inside()[0];

		total += get_attachment_offset(step.input.metas_of_assets, *slot_handle, total, cosm[item_in_slot]);

		current_slot.container_entity = item_in_slot;
	}

	return total;
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

	output.push_back({ first_handle, inventory_slot_id() });
	output.push_back({ second_handle, inventory_slot_id() });

	output.push_back({ first_handle, second_slot });
	output.push_back({ second_handle, first_slot });

	return output;
}

void perform_transfer(
	const item_slot_transfer_request r, 
	const logic_step step
) {
	auto& cosmos = step.cosm;
	const auto transferred_item = cosmos[r.item];
	auto& item = transferred_item.get<components::item>();

	const auto result = query_transfer_result(cosmos, r);

	if (!is_successful(result.result)) {
		LOG("Warning: an item-slot transfer was not successful.");
		return;
	}

	const auto previous_slot = cosmos[item.current_slot];
	const auto target_slot = cosmos[r.target_slot];

	const auto previous_slot_container = previous_slot.get_container();
	const auto target_slot_container = target_slot.get_container();

	const bool is_pickup = result.result == item_transfer_result_type::SUCCESSFUL_PICKUP;
	const bool target_slot_exists = result.result == item_transfer_result_type::SUCCESSFUL_TRANSFER || is_pickup;
	const bool is_drop_request = result.result == item_transfer_result_type::SUCCESSFUL_DROP;

	const auto initial_transform_of_transferred = transferred_item.get_logic_transform();

	//if (result.result == item_transfer_result_type::UNMOUNT_BEFOREHAND) {
	//	ensure(false);
	//	ensure(previous_slot.alive());
	//
	//	//item.request_unmount(r.get_target_slot());
	//	//item.mark_parent_enclosing_containers_for_unmount();
	//
	//	return;
	//}
	
	entity_id target_item_to_stack_with_id;

	if (target_slot_exists) {
		for (const auto potential_stack_target : target_slot.get_items_inside()) {
			if (can_stack_entities(transferred_item, cosmos[potential_stack_target])) {
				target_item_to_stack_with_id = potential_stack_target;
			}
		}
	}

	const bool whole_item_grabbed = item.charges == result.transferred_charges;

	components::transform previous_container_transform;

	if (previous_slot.alive()) {
		previous_container_transform = previous_slot_container.get_logic_transform();

		if (whole_item_grabbed) {
			detail_remove_item(previous_slot, transferred_item);
		}

		if (previous_slot.is_hand_slot()) {
			unset_input_flags_of_orphaned_entity(transferred_item);
		}
	}

	const auto target_item_to_stack_with = cosmos[target_item_to_stack_with_id];

	if (target_item_to_stack_with.alive()) {
		if (whole_item_grabbed) {
			step.transient.messages.post(messages::queue_destruction(transferred_item));
		}
		else {
			item.charges -= result.transferred_charges;
		}

		target_item_to_stack_with.get<components::item>().charges += result.transferred_charges;

		return;
	}

	entity_id grabbed_item_part;

	if (whole_item_grabbed) {
		grabbed_item_part = transferred_item;
	}
	else {
		grabbed_item_part = cosmos.clone_entity(transferred_item);
		item.charges -= result.transferred_charges;
		cosmos[grabbed_item_part].get<components::item>().charges = result.transferred_charges;
	}

	const auto grabbed_item_part_handle = cosmos[grabbed_item_part];

	if (target_slot_exists) {
		detail_add_item(target_slot, grabbed_item_part_handle);
	}

	const auto physics_updater = [previous_container_transform, initial_transform_of_transferred, step](
		const entity_handle descendant, 
		auto... args
	) {
		const auto& cosmos = descendant.get_cosmos();

		const auto slot = cosmos[descendant.get<components::item>().current_slot];
		
		entity_id owner_body = descendant;
		bool should_fixtures_persist = true;
		bool should_body_persist = true;
		components::transform fixtures_offset;
		components::transform force_joint_offset;
		components::transform target_transform = initial_transform_of_transferred;
		bool slot_requests_connection_of_bodies = false;

		if (slot.alive()) {
			should_fixtures_persist = slot.is_physically_connected_until();
			
			if (should_fixtures_persist) {
				const auto first_with_body = slot.get_first_ancestor_with_body_connection();

				fixtures_offset = sum_attachment_offsets(step, descendant.get_address_from_root(first_with_body));
				
				if (slot->physical_behaviour == slot_physical_behaviour::CONNECT_BODIES_BY_JOINT) {
					slot_requests_connection_of_bodies = true;
					force_joint_offset = fixtures_offset;
					target_transform = first_with_body.get_logic_transform() * force_joint_offset;
					fixtures_offset.reset();
				}
				else {
					should_body_persist = false;
					owner_body = first_with_body;
				}
			}
			else {
				should_body_persist = false;
				owner_body = descendant;
			}
		}

		auto def = descendant.get<components::fixtures>().get_raw_component();
		def.offsets_for_created_shapes[colliders_offset_type::ITEM_ATTACHMENT_DISPLACEMENT] = fixtures_offset;
		def.activated = should_fixtures_persist;
		def.owner_body = owner_body;

		descendant.get<components::fixtures>() = def;
		
		auto& rigid_body = descendant.get<components::rigid_body>();
		rigid_body.set_activated(should_body_persist);

		if (should_body_persist) {
			rigid_body.set_transform(target_transform);
			rigid_body.set_velocity({ 0.f, 0.f });
			rigid_body.set_angular_velocity(0.f);
		}

		{
			const auto motor_handle = descendant.get<components::motor_joint>();
			components::motor_joint motor = motor_handle.get_raw_component();

			if (slot_requests_connection_of_bodies) {
				motor.activated = true;  
				motor.target_bodies.at(0) = slot.get_container();
				motor.target_bodies.at(1) = descendant;
				motor.linear_offset = force_joint_offset.pos;
				motor.angular_offset = force_joint_offset.rotation;
				motor.collide_connected = false;
			}
			else {
				motor.activated = false;  
			}

			motor_handle = motor;
		}

		messages::interpolation_correction_request request;
		request.subject = descendant;
		request.set_previous_transform_value = target_transform;
		step.transient.messages.post(request);

		return recursive_callback_result::CONTINUE_AND_RECURSE;
	};

	physics_updater(grabbed_item_part_handle);
	grabbed_item_part_handle.for_each_contained_item_recursive(step.input.metas_of_assets, physics_updater);

	if (is_pickup) {
		const auto target_capability = target_slot_container.get_owning_transfer_capability();

		messages::item_picked_up_message msg;
		msg.subject = target_capability;
		msg.item = grabbed_item_part_handle;

		step.transient.messages.post(msg);
	}
	
	auto& grabbed_item = grabbed_item_part_handle.get<components::item>();

	if (target_slot_exists) {
		if (target_slot->items_need_mounting) {
			grabbed_item.intended_mounting = components::item::MOUNTED;

			if (r.force_immediate_mount) {
				grabbed_item.current_mounting = components::item::MOUNTED;
			}
		}
	}

	if (is_drop_request) {
		ensure(previous_slot_container.alive());

		auto& rigid_body = grabbed_item_part_handle.get<components::rigid_body>();
		
		// LOG_NVPS(rigid_body.velocity());
		// ensure(rigid_body.velocity().is_epsilon());

		rigid_body.set_velocity({ 0.f, 0.f });
		rigid_body.set_angular_velocity(0.f);

		if (r.impulse_applied_on_drop > 0.f) {
			const auto impulse = vec2().set_from_degrees(previous_container_transform.rotation) * r.impulse_applied_on_drop;
			rigid_body.apply_impulse(impulse * rigid_body.get_mass());
		}

		rigid_body.apply_angular_impulse(1.5f * rigid_body.get_mass());
		
		auto& special_physics = grabbed_item_part_handle.get<components::special_physics>();
		special_physics.dropped_or_created_cooldown.set(300, cosmos.get_timestamp());
		special_physics.during_cooldown_ignore_collision_with = previous_slot_container;
	}
}