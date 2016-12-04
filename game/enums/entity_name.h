#pragma once
#include <string>
#include "game/transcendental/entity_handle_declaration.h"

enum class entity_name {
	INVALID,

	ASSAULT_RIFLE,
	BILMER2000,
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

void name_entity(const entity_handle&, const entity_name);
void name_entity(const entity_handle&, const entity_name, const std::wstring& custom_nickname);
