#pragma once
#include "augs/padding_byte.h"
#include "augs/misc/stepped_timing.h"
#include "game/enums/spell_type.h"

#include "augs/misc/stepped_timing.h"

#include "game/assets/texture_id.h"
#include "game/transcendental/entity_handle_declaration.h"
#include "game/transcendental/step_declaration.h"
#include "augs/misc/enum_associative_array.h"

struct spell_data;

struct spell_instance_data {
	augs::stepped_cooldown cast_cooldown;
};

struct spell_appearance {
	assets::texture_id icon = assets::texture_id::INVALID;
	rgba border_col;
};

void set_standard_spell_properties(augs::enum_associative_array<spell_type, spell_data>&);

spell_appearance get_spell_appearance(const spell_type);

bool are_additional_conditions_for_casting_fulfilled(
	const spell_type,
	const const_entity_handle subject
);

void perform_spell_logic(
	const logic_step step,
	const spell_type,
	const entity_handle subject,
	components::sentience& subject_sentience,
	const augs::stepped_timestamp when_casted,
	const augs::stepped_timestamp now
);

std::wstring describe_spell(
	const const_entity_handle,
	const spell_type
);
