#include "electric_shield.h"
#include "game/transcendental/entity_handle.h"
#include "game/transcendental/cosmos.h"
#include "game/components/sentience_component.h"
#include "game/components/particles_existence_component.h"
#include "game/components/sound_existence_component.h"
#include "game/detail/spells/spell_logic_input.h"
#include "game/detail/spells/spell_utils.h"

bool electric_shield_instance::are_additional_conditions_for_casting_fulfilled(const const_entity_handle) const {
	return true;
}

void electric_shield_instance::perform_logic(const spell_logic_input in) {
	const auto& spell_data = std::get<electric_shield>(in.subject.get_cosmos().get_common_state().spells);
	const auto caster_transform = in.subject.get_logic_transform();

	ignite_cast_sparkles(spell_data, in.step, caster_transform, in.subject);
	play_cast_successful_sound(spell_data, in.step, caster_transform, in.subject);

	in.sentience.get<electric_shield_perk_instance>().timing.set_for_duration(static_cast<float>(spell_data.perk_duration_seconds * 1000), in.now); 
}