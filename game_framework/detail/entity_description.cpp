#include "entity_description.h"
#include "entity_system/entity.h"
#include "../components/name_component.h"
#include "../components/gun_component.h"
#include "../components/damage_component.h"
#include "../components/container_component.h"
#include "../components/item_component.h"
#include "../detail/inventory_utils.h"
#include "../detail/inventory_slot.h"
#include "../detail/inventory_slot_id.h"
#include "log.h"
#include "stream.h"

textual_description description_of_entity(augs::entity_id id) {
	return description_by_entity_name(id->get<components::name>().id);
}

std::wstring describe_properties(augs::entity_id id) {
	std::wostringstream result;

	auto* gun = id->find<components::gun>();
	auto* damage = id->find<components::damage>();
	auto* container = id->find<components::container>();
	auto* item = id->find<components::item>();

	if (item) {
		if (item->categories_for_slot_compatibility != 0)
			result << L"[color=vsblue]" << describe_item_compatibility_categories(item->categories_for_slot_compatibility) << L"[/color]\n";
		
		auto total_occupied = format_space_units(calculate_space_occupied_with_children(id));
		auto per_charge = format_space_units(item->space_occupied_per_charge);

		result << "Occupies: [color=vscyan]" << total_occupied << " [/color]";
		
		if (item->charges > 1)
			result << "[color=vsdarkgray](" << per_charge << L" each)[/color]";
		else if(container && total_occupied != per_charge)
			result << "[color=vsdarkgray](" << per_charge << L" if empty)[/color]";

		result << L"\n";
	}

	if (gun) {
		result << L"Muzzle velocity: [color=vscyan]" << (gun->muzzle_velocity.first + gun->muzzle_velocity.second) / 2 
			<< L"[/color]\nDamage multiplier: [color=vscyan]" << std::fixed << std::setprecision(1) << gun->damage_multiplier << L"[/color]\n";
	}

	if (damage) {
		result << L"Base damage: [color=vscyan]" << damage->amount << L"[/color]\n";

		if (damage->constrain_distance)
			result << L"Max distance: [color=vscyan]" << damage->max_distance << L"[/color]\n";
		if (damage->constrain_lifetime)
			result << L"Max lifetime: [color=vscyan]" << damage->max_lifetime_ms << L" ms[/color]\n";
	}

	if (id.has(slot_function::ITEM_DEPOSIT)) {
		auto depo = id[slot_function::ITEM_DEPOSIT];

		auto children_space = format_space_units(depo->calculate_free_space_with_children());
		auto with_parents_space = format_space_units(depo.calculate_free_space_with_parent_containers());

		result << L"Deposit space: [color=vsgreen]" << format_space_units(depo.calculate_free_space_with_parent_containers()) << L"[/color]/";

		if (children_space != with_parents_space)
			result << L"[color=vsyellow]" << format_space_units(depo->calculate_free_space_with_children()) << L"[/color]/";
			
		result << L"[color=vscyan]" << format_space_units(depo->space_available) << L"[/color]\n";
	}

	std::wstring out;

	if (id.has(sub_entity_name::BULLET_ROUND_DEFINITION)) {
		out = result.str() + describe_properties(id[sub_entity_name::BULLET_ROUND_DEFINITION]);
		return out;
	}
	else {
		out = result.str();
		return out.substr(0, out.length() - 1);
	}
}

std::wstring describe_slot(inventory_slot_id id) {
	auto text = describe_slot_function(id.type);

	auto catcolor = id->for_categorized_items_only ? L"violet" : L"vsblue";

	return text.name + L"\n[color=vslightgray]Allows: [/color][color=" + catcolor + L"]" + describe_item_compatibility_categories(id->category_allowed) + L"[/color][color=vsdarkgray]\n" +
		text.details + L"[/color]";
}

std::wstring describe_entity(augs::entity_id id) {
	auto desc = description_of_entity(id);
	auto properties = describe_properties(id);
	if (!properties.empty()) properties += L"\n";

	return L"[color=white]" + desc.name + L"[/color]\n" + properties + L"[color=vsdarkgray]" + desc.details + L"[/color]";
}