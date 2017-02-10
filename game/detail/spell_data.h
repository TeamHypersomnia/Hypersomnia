#pragma once
#include "augs/padding_byte.h"
#include "augs/misc/stepped_timing.h"
#include "game/enums/spell_type.h"

#include "augs/misc/constant_size_vector.h"
#include "augs/misc/stepped_timing.h"

#include "game/assets/texture_id.h"
#include "game/transcendental/entity_handle_declaration.h"

struct spell_data {
	unsigned personal_electricity_required = 40;
	unsigned cooldown_ms = 5000;
	unsigned perk_seconds = 0;
	augs::constant_size_wstring<32> incantation;
};

struct spell_instance_data {
	augs::stepped_cooldown cast_cooldown;
};

struct spell_appearance {
	assets::texture_id icon = assets::texture_id::INVALID;
	rgba border_col;
};

spell_data get_spell_data(const spell_type);
spell_appearance get_spell_appearance(const spell_type);

void do_spell_callback(
	const spell_type,
	const entity_handle subject,
	const augs::stepped_timestamp when_casted,
	const augs::stepped_timestamp now
);

std::wstring describe_spell(
	const const_entity_handle,
	const spell_type
);
