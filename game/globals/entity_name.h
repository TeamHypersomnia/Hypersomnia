#pragma once
#include "entity_system/entity_id.h"

enum class entity_name {
	ASSAULT_RIFLE,
	MAGAZINE,
	GREEN_CHARGE,
	PINK_CHARGE,
	CYAN_CHARGE,
	TRUCK,
	MOTORCYCLE,
	VIOLET_BACKPACK,
	SUPPRESSOR,
	CRATE,
	SENTIENCE,
	PERSON,
	PISTOL,
	SUBMACHINE,
	URBAN_CYAN_MACHETE,
};

void name_entity(augs::entity_id, entity_name);
void name_entity(augs::entity_id, entity_name, std::wstring custom_nickname);
