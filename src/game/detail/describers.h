#pragma once
#include <string>
#include <map>

#include "game/transcendental/entity_id.h"

#include "game/detail/inventory/inventory_slot_handle_declaration.h"
#include "game/transcendental/entity_handle_declaration.h"
#include "game/enums/slot_function.h"
#include "game/enums/item_category.h"

#include "game/transcendental/entity_flavour_id.h"
#include "game/common_state/entity_flavours.h"

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

entity_name_str get_bbcoded_entity_name(const const_entity_handle maybe_overridden_by_nickname);

const entity_name_str& get_bbcoded_entity_description(const const_entity_handle);

entity_name_str get_bbcoded_item_categories(const item_category_flagset& flags);

entity_name_str get_bbcoded_slot_function_name(const slot_function);
entity_name_str get_bbcoded_slot_function_description(const slot_function);

entity_name_str get_bbcoded_slot_details(const const_inventory_slot_handle);
entity_name_str get_bbcoded_entity_details(const const_entity_handle);

template <class entity_handle_type, class T>
entity_name_str get_bbcoded_spell_description(
	const entity_handle_type subject,
	const T& spell
) {
	const auto properties = typesafe_sprintf(
		"Incantation: [color=yellow]%x[/color]\nPE to cast: [color=vscyan]%x[/color]\nCooldown: [color=vscyan]%x[/color]",
		spell.appearance.incantation, 
		spell.common.personal_electricity_required, 
		spell.common.cooldown_ms
	);

	return spell.appearance.name + "\n" + properties + "\n" + spell.appearance.description;
}

using name_accumulator = std::map<entity_name_str, std::size_t>;

entity_name_str describe_names_of(const name_accumulator& counts);

template <class F>
entity_name_str describe_names_of(F for_each_name) {
	thread_local name_accumulator counts;
	counts.clear();

	for_each_name([&](const auto& name){
		++counts[name];
	});

	return describe_names_of(counts);
}
