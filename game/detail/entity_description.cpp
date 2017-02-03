#include "entity_description.h"
#include "game/transcendental/entity_handle.h"
#include "game/detail/inventory_slot_handle.h"
#include "game/components/name_component.h"
#include "game/components/melee_component.h"
#include "game/components/gun_component.h"
#include "game/components/damage_component.h"
#include "game/components/container_component.h"
#include "game/components/sentience_component.h"
#include "game/components/item_component.h"
#include "game/detail/inventory_utils.h"
#include "game/detail/inventory_slot.h"
#include "game/detail/inventory_slot_id.h"
#include "game/transcendental/cosmos.h"
#include "augs/log.h"
#include <iomanip>

textual_description description_of_entity(const const_entity_handle id) {
	const auto& name = id.get<components::name>();
	
	auto result = description_by_entity_name(name.id);

	if (name.custom_nickname) {
		result.name = name.get_nickname();
	}

	return result;
}

std::wstring describe_properties(const const_entity_handle id) {
	std::wostringstream result;

	const auto* const melee = id.find<components::melee>();
	const auto* const gun = id.find<components::gun>();
	const auto* const damage = id.find<components::damage>();
	const auto* const container = id.find<components::container>();
	const auto* const item = id.find<components::item>();

	if (item) {
		if (item->categories_for_slot_compatibility.any()) {
			result << L"[color=vsblue]" << describe_item_compatibility_categories(item->categories_for_slot_compatibility) << L"[/color]\n";
		}
		
		const auto total_occupied = format_space_units(calculate_space_occupied_with_children(id));
		const auto per_charge = format_space_units(item->space_occupied_per_charge);

		result << "Occupies: [color=vscyan]" << total_occupied << " [/color]";
		
		if (item->charges > 1) {
			result << "[color=vsdarkgray](" << per_charge << L" each)[/color]";
		}
		else if (container && total_occupied != per_charge) {
			result << "[color=vsdarkgray](" << per_charge << L" if empty)[/color]";
		}

		result << L"\n";
	}

	if (gun) {
		result << L"Muzzle velocity: [color=vscyan]" << (gun->muzzle_velocity.first + gun->muzzle_velocity.second) / 2 
			<< L"[/color]\nDamage multiplier: [color=vscyan]" << std::fixed << std::setprecision(1) << gun->damage_multiplier << L"[/color]\n";
	}

	if (damage) {
		if (damage->amount > 0) {
			result << L"Base damage: [color=vscyan]" << damage->amount << L"[/color]\n";
		}
		else if (damage->amount < 0) {
			result << L"Restores health: [color=vscyan]" << -damage->amount << L"[/color]\n";
		}

		if (damage->constrain_distance) {
			result << L"Max distance: [color=vscyan]" << damage->max_distance << L"[/color]\n";
		}
		if (damage->constrain_lifetime) {
			result << L"Max lifetime: [color=vscyan]" << damage->max_lifetime_ms << L" ms[/color]\n";
		}
	}

	if (melee) {
		result << L"Swing duration: [color=vscyan]" << melee->swings[0].duration_ms << L" ms[/color]\n";
		result << L"Swing cooldown: [color=vscyan]" << melee->swings[0].cooldown_ms << L" ms[/color]\n";
	}

	const auto& depo = id[slot_function::ITEM_DEPOSIT];

	if (depo.alive()) {
		const auto children_space = format_space_units(depo.calculate_local_free_space());
		const auto with_parents_space = format_space_units(depo.calculate_real_free_space());

		result << L"Deposit space: [color=vsgreen]" << format_space_units(depo.calculate_real_free_space()) << L"[/color]/";

		if (children_space != with_parents_space)
			result << L"[color=vsyellow]" << format_space_units(depo.calculate_local_free_space()) << L"[/color]/";
			
		result << L"[color=vscyan]" << format_space_units(depo->space_available) << L"[/color]\n";
	}

	std::wstring out;

	const auto& bullet_round = id[sub_entity_name::BULLET_ROUND];

	if (bullet_round.alive()) {
		out = result.str() + describe_properties(bullet_round);
		return out;
	}
	else {
		out = result.str();
		return out.substr(0, out.length() - 1);
	}
}

std::wstring describe_slot(const const_inventory_slot_handle& id) {
	const auto text = describe_slot_function(id.get_id().type);
	const auto catcolor = id->for_categorized_items_only ? L"violet" : L"vsblue";

	return text.name + L"\n[color=vslightgray]Allows: [/color][color=" + catcolor + L"]" + describe_item_compatibility_categories(id->get_allowed_categories()) + L"[/color][color=vsdarkgray]\n" +
		text.details + L"[/color]";
}

std::wstring describe_entity(const const_entity_handle id) {
	const auto desc = description_of_entity(id);
	auto properties = describe_properties(id);
	
	if (!properties.empty()) {
		properties += L"\n";
	}

	return L"[color=white]" + desc.name + L"[/color]\n" + properties + L"[color=vsdarkgray]" + desc.details + L"[/color]";
}

std::wstring describe_sentience_meter(
	const const_entity_handle subject,
	const sentience_meter_type type
) {
	const auto& sentience = subject.get<components::sentience>();
	const auto& meter = sentience.get(type);

	if (type == sentience_meter_type::HEALTH) {
		return typesafe_sprintf(L"[color=red]Health points:[/color] %x/%x\n[color=vsdarkgray]Stability of the physical body.[/color]", meter.value, meter.maximum);
	}

	if (type == sentience_meter_type::PERSONAL_ELECTRICITY) {
		return typesafe_sprintf(L"[color=cyan]Personal electricity:[/color] %x/%x\n[color=vsdarkgray]Mind-programmable matter.[/color]", meter.value, meter.maximum);
	}

	if (type == sentience_meter_type::CONSCIOUSNESS) {
		return typesafe_sprintf(L"[color=orange]Consciousness:[/color] %x/%x\n[color=vsdarkgray]Attunement of soul with the body.[/color]", meter.value, meter.maximum);
	}

	else return L"Unknown problem";
}