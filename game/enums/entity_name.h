#pragma once
#include <string>
#include "game/transcendental/entity_handle_declaration.h"

enum class entity_name {
	INVALID,

	ASSAULT_RIFLE,
	BILMER2000,
	KEK9,
	MAGAZINE,
	GREEN_CHARGE,
	PINK_CHARGE,
	CYAN_CHARGE,
	TRUCK,
	JMIX114,
	VIOLET_BACKPACK,
	SUPPRESSOR,
	CRATE,
	SENTIENCE,
	PERSON,
	PISTOL,
	SUBMACHINE,
	URBAN_CYAN_MACHETE,
	FORCE_GRENADE,
	PED_GRENADE,
	INTERFERENCE_GRENADE,
	AMPLIFIER_ARM
};

void name_entity(const entity_handle, const entity_name);
void name_entity(const entity_handle, const entity_name, const std::wstring& custom_nickname);
