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

template <class C, class Container>
std::wstring describe_names_of(const Container& all_entities, const C& cosm) {
	if (all_entities.empty()) {
		return L"";
	}

	auto quoted = [](const auto& s) {
#if 0
		return L'"' + s + L'"';
#endif
		return s;
	};

	if (all_entities.size() == 1) {
		return quoted(cosm[(*all_entities.begin())].get_name());
	}

	/* More than one. */

	thread_local std::map<std::wstring, std::size_t> counts;
	counts.clear();

	for (const auto& e : all_entities) {
		++counts[cosm[e].get_name()];
	}

	std::wstring result;

	std::size_t total = 0;

	for (const auto& c : counts) {
		result += typesafe_sprintf(L"%x of %x, ", c.second, quoted(c.first));
		total += c.second;
	}

	if (counts.size() > 1) {
		result = typesafe_sprintf(L"%x: ", total) + result;
	}

	result.pop_back();
	result.pop_back();

	return result;
}
