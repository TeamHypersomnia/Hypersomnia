#pragma once
#include <string>
#include "game_framework/globals/entity_name.h"
#include "entity_system/entity_id.h"

struct entity_description {
	std::wstring name;
	std::wstring details;
} description_by_entity_name(entity_name),
description_of_entity(augs::entity_id)
;
