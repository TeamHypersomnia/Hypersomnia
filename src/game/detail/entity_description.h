#pragma once
#include <string>

#include "game/transcendental/entity_id.h"

#include "game/detail/inventory/inventory_slot_handle_declaration.h"
#include "game/transcendental/entity_handle_declaration.h"
#include "game/enums/slot_function.h"
#include "game/enums/item_category.h"
#include "game/enums/sentience_meter_type.h"
#include "game/assets/spell_id.h"
#include "game/components/name_component.h"

/*
	Example description:

	0. Force grenade
	1. Occupies: 0.6
	2. Deals high damage to health of personnel
	3. and armor.

	Parts:

	0: entity name
	1: entity properties
	2-3: entity details
	0-3 (the whole): entity description
*/

entity_name_type get_bbcoded_entity_name(const const_entity_handle maybe_overridden_by_nickname);
std::wstring get_bbcoded_entity_name_details(const const_entity_handle);

std::wstring get_bbcoded_item_categories(const item_category_flagset& flags);

std::wstring get_bbcoded_slot_function_name(const slot_function);
std::wstring get_bbcoded_slot_function_details(const slot_function);

std::wstring get_bbcoded_slot_description(const const_inventory_slot_handle);
std::wstring get_bbcoded_entity_description(const const_entity_handle);

std::wstring get_bbcoded_sentience_meter_description(
	const const_entity_handle,
	const sentience_meter_type
);

std::wstring get_bbcoded_spell_description(
	const const_entity_handle,
	const assets::spell_id
);