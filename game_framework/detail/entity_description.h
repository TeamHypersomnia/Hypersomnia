#pragma once
#include <string>
#include "game_framework/globals/entity_name.h"
#include "entity_system/entity_id.h"

struct textual_description {
	std::wstring name;
	std::wstring details;
} description_by_entity_name(entity_name),
description_of_entity(augs::entity_id)
;

std::wstring describe_properties(augs::entity_id);

std::wstring describe_item_compatibility_categories(unsigned flags);
textual_description describe_slot_function(slot_function);

std::wstring describe_slot(inventory_slot_id);