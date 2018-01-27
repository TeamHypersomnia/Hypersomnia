#pragma once
#include <string>

#include "game/transcendental/entity_id.h"

#include "game/detail/inventory/inventory_slot_handle_declaration.h"
#include "game/transcendental/entity_handle_declaration.h"
#include "game/enums/slot_function.h"
#include "game/enums/item_category.h"

#include "game/transcendental/entity_flavour_id.h"
#include "game/common_state/entity_types.h"

/*
	Example details:

	0. Force grenade
	1. Occupies: 0.6
	2. Deals high damage to health of personnel
	3. and armor.

	Parts:

	0: entity name
	1: entity properties
	2-3: entity description
	0-3 (the whole): entity details
*/

entity_name_type get_bbcoded_entity_name(const const_entity_handle maybe_overridden_by_nickname);

const entity_description_type& get_bbcoded_entity_description(const const_entity_handle);

std::wstring get_bbcoded_item_categories(const item_category_flagset& flags);

std::wstring get_bbcoded_slot_function_name(const slot_function);
std::wstring get_bbcoded_slot_function_description(const slot_function);

std::wstring get_bbcoded_slot_details(const const_inventory_slot_handle);
std::wstring get_bbcoded_entity_details(const const_entity_handle);

template <class entity_handle_type, class T>
std::wstring get_bbcoded_spell_description(
	const entity_handle_type subject,
	const T& spell
) {
	const auto properties = typesafe_sprintf(
		L"Incantation: [color=yellow]%x[/color]\nPE to cast: [color=vscyan]%x[/color]\nCooldown: [color=vscyan]%x[/color]",
		spell.appearance.incantation, 
		spell.common.personal_electricity_required, 
		spell.common.cooldown_ms
	);

	return spell.appearance.name + L"\n" + properties + L"\n" + spell.appearance.description;
}