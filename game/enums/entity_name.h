#pragma once
#include <string>
#include "game/entity_handle_declaration.h"

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
	CORPSE,
	URBAN_CYAN_MACHETE,
};

void name_entity(entity_handle, entity_name);
void name_entity(entity_handle, entity_name, std::wstring custom_nickname);
