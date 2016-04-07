#pragma once
#include "entity_system/entity_id.h"

enum class entity_name {
	ASSAULT_RIFLE,
	MAGAZINE,
	PINK_CHARGE,
	CYAN_CHARGE,
	TRUCK,
	MOTORCYCLE,
	VIOLET_BACKPACK,
	SUPPRESSOR
};

void name_entity(augs::entity_id, entity_name);