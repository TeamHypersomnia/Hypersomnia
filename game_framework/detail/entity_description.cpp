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
		result << L"Muzzle velocity: " << (gun->muzzle_velocity.first + gun->muzzle_velocity.second) / 2 
			<< L"\nDamage multiplier: " << std::fixed << std::setprecision(1) << gun->damage_multiplier;
	}

	if (damage) {
		result << L"Base damage: " << damage->amount;

		if (damage->constrain_distance)
			result << L"\nMax distance: " << damage->max_distance;
		if (damage->constrain_lifetime)
			result << L"\nMax lifetime: " << damage->max_lifetime_ms << L" ms";
	}

	return result.str();
}