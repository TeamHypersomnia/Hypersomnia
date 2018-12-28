#include "fury_of_the_aeons.h"
#include "game/cosmos/entity_handle.h"
#include "game/cosmos/cosmos.h"
#include "game/components/sentience_component.h"
#include "game/detail/view_input/particle_effect_input.h"
#include "game/detail/view_input/sound_effect_input.h"
#include "game/detail/spells/spell_logic_input.h"
#include "game/detail/spells/spell_utils.h"

bool fury_of_the_aeons_instance::are_additional_conditions_for_casting_fulfilled(const const_entity_handle) const {
	return true;
}

void fury_of_the_aeons_instance::perform_logic(const spell_logic_input in) {
	const auto subject = in.get_subject();
	const auto& spell_data = std::get<fury_of_the_aeons>(subject.get_cosmos().get_common_significant().spells);
	const auto caster_transform = subject.get_logic_transform();

	ignite_cast_sparkles(spell_data, in.step, subject);
	play_cast_successful_sound(spell_data, in.step, subject);

	auto cause = damage_cause(subject);
	cause.spell = in.this_id;

	spell_data.explosion.instantiate(in.step, caster_transform, cause, predictable_only_by(subject));
}