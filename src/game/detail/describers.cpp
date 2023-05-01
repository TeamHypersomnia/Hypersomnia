#include "game/detail/describers.h"

#include "augs/templates/container_templates.h"
#include "game/cosmos/entity_handle.h"

#include "game/detail/inventory/inventory_slot_handle.h"
#include "game/components/melee_component.h"
#include "game/components/gun_component.h"
#include "game/components/missile_component.h"
#include "game/components/container_component.h"
#include "game/components/sentience_component.h"
#include "game/components/item_component.h"
#include "game/components/cartridge_component.h"
#include "game/detail/inventory/inventory_slot.h"
#include "game/detail/inventory/inventory_slot_id.h"
#include "game/cosmos/cosmos.h"

#include "game/detail/inventory/inventory_utils.h"

entity_name_str describe_names_of(const name_accumulator& counts) {
	if (counts.empty()) {
		return "";
	}

	auto quoted = [](const auto& s) {
#if 0
		return '"' + s + '"';
#endif
		return s;
	};

	const auto total = accumulate_mapped_values(counts);

	if (total == 1) {
		entity_name_str result;

		for (const auto& c : counts) {
			result = quoted(c.first);
		}

		return result;
	}

	entity_name_str result;

	for (const auto& c : counts) {
		result += typesafe_sprintf("%x of %x, ", c.second, quoted(c.first));
	}

	if (counts.size() > 1) {
		/* Add total count at the beginning if there are two or more distinct names */
		result = typesafe_sprintf("%x: ", total) + result;
	}

	/* Trim the trailing comma and space */
	result.pop_back();
	result.pop_back();

	return result;
}

entity_name_str get_bbcoded_entity_name(const const_entity_handle maybe_overridden_by_nickname) {
	return maybe_overridden_by_nickname.get_name();
}

const entity_name_str& get_bbcoded_entity_description(const const_entity_handle handle) {
	return handle.get_description();
}

entity_name_str get_bbcoded_entity_properties(const const_entity_handle id) {
	std::ostringstream result;

	const auto* const gun = id.find<components::gun>();
	const auto* const gun_def = id.find<invariants::gun>();
	const auto* const missile = id.find<invariants::missile>();
	const auto* const container = id.find<invariants::container>();

	const auto* const item_def = id.find<invariants::item>();
	const auto item = id.find<components::item>();

	if (item && item_def) {
		result << "[color=vsblue]" << get_bbcoded_item_categories(item_def->categories_for_slot_compatibility) << "[/color]\n";
		
		const auto total_occupied = format_space_units(calc_space_occupied_with_children(id));
		const auto per_charge = format_space_units(item_def->space_occupied_per_charge);

		result << "Occupies: [color=vscyan]" << total_occupied << " [/color]";
		
		if (item->get_charges() > 1) {
			result << "[color=vsdarkgray](" << per_charge << " each)[/color]";
		}
		else if (container && total_occupied != per_charge) {
			result << "[color=vsdarkgray](" << per_charge << " if empty)[/color]";
		}

		result << "\n";
	}

	if (gun) {
		if (const auto flavour = gun_def->magic_missile_flavour; flavour.is_set()) {
			result << typesafe_sprintf("Muzzle velocity: [color=vscyan]%x[/color]\nAmplification multiplier: [color=vscyan]%x[/color]\n", 
				(gun_def->muzzle_velocity.first + gun_def->muzzle_velocity.second) / 2, gun_def->damage_multiplier);
		}
		else {
			result << typesafe_sprintf("Muzzle velocity: [color=vscyan]%x[/color]\nDamage multiplier: [color=vscyan]%x[/color]\n",
				(gun_def->muzzle_velocity.first + gun_def->muzzle_velocity.second) / 2, gun_def->damage_multiplier);
		}
	}

	if (missile) {
		if (missile->damage.base > 0) {
			result << "Base missile: [color=vscyan]" << missile->damage.base << "[/color]\n";
		}
		else if (missile->damage.base < 0) {
			result << "Restores health: [color=vscyan]" << -missile->damage.base << "[/color]\n";
		}

		if (missile->constrain_lifetime) {
			result << "Max lifetime: [color=vscyan]" << missile->max_lifetime_ms << " ms[/color]\n";
		}
	}

	//if (melee) {
	//	result << "Swing duration: [color=vscyan]" << melee->swings[0].duration_ms << " ms[/color]\n";
	//	result << "Swing cooldown: [color=vscyan]" << melee->swings[0].cooldown_ms << " ms[/color]\n";
	//}

	const auto& depo = id[slot_function::ITEM_DEPOSIT];

	if (depo.alive()) {
		const auto children_space = format_space_units(depo.calc_local_space_available());
		const auto with_parents_space = format_space_units(depo.calc_real_space_available());

		result << "Deposit space: [color=vsgreen]" << format_space_units(depo.calc_real_space_available()) << "[/color]/";

		if (children_space != with_parents_space)
			result << "[color=vsyellow]" << format_space_units(depo.calc_local_space_available()) << "[/color]/";
			
		result << "[color=vscyan]" << format_space_units(depo->space_available) << "[/color]\n";
	}

	entity_name_str out;

	// TODO: describe cartridge invariant and describe types inside
	
	out = result.str();
	return out.substr(0, out.length() - 1);
}

entity_name_str get_bbcoded_slot_details(const const_inventory_slot_handle id) {
	const auto name = get_bbcoded_slot_function_name(id.get_id().type);
	const auto description = get_bbcoded_slot_function_description(id.get_id().type);

	const auto catcolor = id->category_allowed == item_category::GENERAL ? "vsblue" : "violet";

	return name + "\n[color=vslightgray]Allows: [/color][color=" + catcolor + "]" + get_bbcoded_item_categories(id->get_allowed_categories()) + "[/color][color=vsdarkgray]\n" +
		description + "[/color]";
}

entity_name_str get_bbcoded_entity_details(const const_entity_handle id) {
	const auto name = get_bbcoded_entity_name(id);
	const auto description = get_bbcoded_entity_description(id);

	auto properties = get_bbcoded_entity_properties(id);
	
	if (!properties.empty()) {
		properties += "\n";
	}

	return "[color=white]" + name + "[/color]\n" + properties + "[color=vsdarkgray]" + description + "[/color]";
}