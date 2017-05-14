#include "entity_description.h"
#include "game/transcendental/entity_handle.h"
#include "game/detail/inventory/inventory_slot_handle.h"
#include "game/components/name_component.h"
#include "game/components/melee_component.h"
#include "game/components/gun_component.h"
#include "game/components/damage_component.h"
#include "game/components/container_component.h"
#include "game/components/sentience_component.h"
#include "game/components/item_component.h"
#include "game/components/catridge_component.h"
#include "game/detail/inventory/inventory_utils.h"
#include "game/detail/inventory/inventory_slot.h"
#include "game/detail/inventory/inventory_slot_id.h"
#include "game/transcendental/cosmos.h"
#include "augs/log.h"
#include <iomanip>

std::wstring get_bbcoded_entity_name(const const_entity_handle maybe_overridden_by_nickname) {
	const auto& name = maybe_overridden_by_nickname.get<components::name>();

	if (name.custom_nickname) {
		return name.get_nickname();
	}
	else {
		return get_bbcoded_entity_name(name.id);
	}
}

std::wstring get_bbcoded_entity_name_details(const const_entity_handle id) {
	return get_bbcoded_entity_name_details(id.get<components::name>().id);
}

std::wstring get_bbcoded_entity_properties(const const_entity_handle id) {
	std::wostringstream result;

	const auto& cosmos = id.get_cosmos();

	const auto* const melee = id.find<components::melee>();
	const auto* const gun = id.find<components::gun>();
	const auto* const damage = id.find<components::damage>();
	const auto* const container = id.find<components::container>();
	const auto* const item = id.find<components::item>();

	if (item) {
		result << L"[color=vsblue]" << get_bbcoded_item_categories(item->categories_for_slot_compatibility) << L"[/color]\n";
		
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
		if (cosmos[gun->magic_missile_definition].alive()) {
			result << typesafe_sprintf(L"Muzzle velocity: [color=vscyan]%x[/color]\nAmplification multiplier: [color=vscyan]%x[/color]\n", 
				(gun->muzzle_velocity.first + gun->muzzle_velocity.second) / 2, gun->damage_multiplier);
		}
		else {
			result << typesafe_sprintf(L"Muzzle velocity: [color=vscyan]%x[/color]\nDamage multiplier: [color=vscyan]%x[/color]\n",
				(gun->muzzle_velocity.first + gun->muzzle_velocity.second) / 2, gun->damage_multiplier);
		}
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

	//if (melee) {
	//	result << L"Swing duration: [color=vscyan]" << melee->swings[0].duration_ms << L" ms[/color]\n";
	//	result << L"Swing cooldown: [color=vscyan]" << melee->swings[0].cooldown_ms << L" ms[/color]\n";
	//}

	const auto& depo = id[slot_function::ITEM_DEPOSIT];

	if (depo.alive()) {
		const auto children_space = format_space_units(depo.calculate_local_space_available());
		const auto with_parents_space = format_space_units(depo.calculate_real_space_available());

		result << L"Deposit space: [color=vsgreen]" << format_space_units(depo.calculate_real_space_available()) << L"[/color]/";

		if (children_space != with_parents_space)
			result << L"[color=vsyellow]" << format_space_units(depo.calculate_local_space_available()) << L"[/color]/";
			
		result << L"[color=vscyan]" << format_space_units(depo->space_available) << L"[/color]\n";
	}

	std::wstring out;

	if (id.has<components::catridge>()) {
		const auto& bullet_round = id[child_entity_name::CATRIDGE_BULLET];

		if (bullet_round.alive()) {
			out = result.str() + get_bbcoded_entity_properties(bullet_round);
			return out;
		}
	}
	
	out = result.str();
	return out.substr(0, out.length() - 1);
}

std::wstring get_bbcoded_slot_description(const const_inventory_slot_handle id) {
	const auto name = get_bbcoded_slot_function_name(id.get_id().type);
	const auto details = get_bbcoded_slot_function_details(id.get_id().type);

	const auto catcolor = id->category_allowed == item_category::GENERAL ? L"vsblue" : L"violet";

	return name + L"\n[color=vslightgray]Allows: [/color][color=" + catcolor + L"]" + get_bbcoded_item_categories(id->get_allowed_categories()) + L"[/color][color=vsdarkgray]\n" +
		details + L"[/color]";
}

std::wstring get_bbcoded_entity_description(const const_entity_handle id) {
	const auto name = get_bbcoded_entity_name(id);
	const auto details = get_bbcoded_entity_name_details(id);

	auto properties = get_bbcoded_entity_properties(id);
	
	if (!properties.empty()) {
		properties += L"\n";
	}

	return L"[color=white]" + name + L"[/color]\n" + properties + L"[color=vsdarkgray]" + details + L"[/color]";
}

std::wstring get_bbcoded_sentience_meter_description(
	const const_entity_handle subject,
	const sentience_meter_type type
) {
	const auto& cosmos = subject.get_cosmos();

	const auto dt = cosmos.get_fixed_delta();
	const auto now = cosmos.get_timestamp();

	const auto& sentience = subject.get<components::sentience>();
	const auto value = sentience.get_value(type, now, dt);
	const auto maximum = sentience.get_maximum_value(type); 

	if (type == sentience_meter_type::HEALTH) {
		return typesafe_sprintf(L"[color=red]Health points:[/color] %x/%x\n[color=vsdarkgray]Stability of the physical body.[/color]", value, maximum);
	}

	if (type == sentience_meter_type::PERSONAL_ELECTRICITY) {
		return typesafe_sprintf(L"[color=cyan]Personal electricity:[/color] %x/%x\n[color=vsdarkgray]Mind-programmable matter.[/color]", value, maximum);
	}

	if (type == sentience_meter_type::CONSCIOUSNESS) {
		return typesafe_sprintf(L"[color=orange]Consciousness:[/color] %x/%x\n[color=vsdarkgray]Attunement of soul with the body.[/color]", value, maximum);
	}

	if (type == sentience_meter_type::HASTE) {
		return typesafe_sprintf(L"[color=green]Haste[/color]\n[color=vsdarkgray]You move faster.[/color]");
	}

	if (type == sentience_meter_type::ELECTRIC_SHIELD) {
		return typesafe_sprintf(L"[color=turquoise]Electric shield[/color]\n[color=vsdarkgray]Damage is absorbed by [/color][color=cyan]Personal Electricity[/color][color=vsdarkgray] instead of [/color][color=red]Health[/color][color=vsdarkgray].[/color]");
	}

	else return L"Unknown problem";
}

std::wstring get_bbcoded_spell_description(
	const const_entity_handle caster,
	const assets::spell_id spell
) {
	const auto& manager = get_assets_manager();

	const auto spell_data = manager[spell];

	const auto properties = typesafe_sprintf(
		L"Incantation: [color=yellow]%x[/color]\nPE to cast: [color=vscyan]%x[/color]\nCooldown: [color=vscyan]%x[/color]",
		std::wstring(spell_data.incantation), 
		spell_data.logical.personal_electricity_required, 
		spell_data.logical.cooldown_ms
	);

	return spell_data.spell_name + L"\n" + properties + L"\n" + spell_data.spell_description;
}