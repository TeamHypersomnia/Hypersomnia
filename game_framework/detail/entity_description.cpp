#include "entity_description.h"
#include "entity_system/entity.h"
#include "../components/name_component.h"
#include "../components/gun_component.h"
#include "../components/damage_component.h"
#include "../components/container_component.h"
#include "../detail/inventory_utils.h"
#include "../detail/inventory_slot.h"
#include "../detail/inventory_slot_id.h"
#include "log.h"
#include "stream.h"

entity_description description_of_entity(augs::entity_id id) {
	return description_by_entity_name(id->get<components::name>().id);
}

std::wstring describe_properties(augs::entity_id id) {
	if (id.has(sub_entity_name::BULLET_ROUND_DEFINITION))
		return describe_properties(id[sub_entity_name::BULLET_ROUND_DEFINITION]);

	std::wostringstream result;

	auto* gun = id->find<components::gun>();
	auto* damage = id->find<components::damage>();
	auto* container = id->find<components::container>();

	if (gun) {
		result << L"Muzzle velocity: [color=vscyan]" << (gun->muzzle_velocity.first + gun->muzzle_velocity.second) / 2 
			<< L"[/color]\nDamage multiplier: [color=vscyan]" << std::fixed << std::setprecision(1) << gun->damage_multiplier << L"[/color]";
	}

	if (damage) {
		result << L"Base damage: [color=vscyan]" << damage->amount << L"[/color]";

		if (damage->constrain_distance)
			result << L"\nMax distance: [color=vscyan]" << damage->max_distance << L"[/color]";
		if (damage->constrain_lifetime)
			result << L"\nMax lifetime: [color=vscyan]" << damage->max_lifetime_ms << L" ms[/color]";
	}

	if (id.has(slot_function::ITEM_DEPOSIT)) {
		auto depo = id[slot_function::ITEM_DEPOSIT];

		auto children_space = format_space_units(depo->calculate_free_space_with_children());
		auto with_parents_space = format_space_units(depo.calculate_free_space_with_parent_containers());

		result << L"Deposit space: [color=vsgreen]" << format_space_units(depo.calculate_free_space_with_parent_containers()) << L"[/color]/";

		if (children_space != with_parents_space)
			result << L"[color=vsyellow]" << format_space_units(depo->calculate_free_space_with_children()) << L"[/color]/";
			
		result << L"[color=vscyan]" << format_space_units(depo->space_available) << L"[/color]";
	}

	return result.str();
}