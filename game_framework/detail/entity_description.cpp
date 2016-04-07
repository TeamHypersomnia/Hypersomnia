#include "entity_description.h"
#include "entity_system/entity.h"
#include "../components/name_component.h"
#include "../components/gun_component.h"
#include "../components/damage_component.h"
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

	return result.str();
}