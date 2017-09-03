#include "fury_of_the_aeons.h"
#include "game/transcendental/entity_handle.h"
#include "game/transcendental/cosmos.h"
#include "game/components/sentience_component.h"
#include "game/components/particles_existence_component.h"
#include "game/components/sound_existence_component.h"
#include "game/detail/spells/spell_logic_input.h"
#include "game/detail/spells/spell_utils.h"

bool fury_of_the_aeons_instance::are_additional_conditions_for_casting_fulfilled(const const_entity_handle) const {
	return true;
}

void fury_of_the_aeons_instance::perform_logic(const spell_logic_input in) {
	const auto& spell_data = std::get<fury_of_the_aeons>(in.subject.get_cosmos().get_common_state().spells);
	const auto caster_transform = in.subject.get_logic_transform();

	ignite_cast_sparkles(spell_data, in.step, caster_transform, in.subject);
	play_cast_successful_sound(spell_data, in.step, caster_transform, in.subject);

	in.sentience.shake_for_ms = 400.f;
	in.sentience.time_of_last_shake = in.now;

	spell_data.explosion.instantiate(in.step, caster_transform, in.subject);
}