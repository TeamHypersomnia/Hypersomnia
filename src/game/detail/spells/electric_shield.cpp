#include "electric_shield.h"
#include "game/cosmos/entity_handle.h"
#include "game/cosmos/cosmos.h"
#include "game/components/sentience_component.h"
#include "game/detail/view_input/particle_effect_input.h"
#include "game/detail/view_input/sound_effect_input.h"
#include "game/detail/spells/spell_logic_input.h"
#include "game/detail/spells/spell_utils.h"

bool electric_shield_instance::are_additional_conditions_for_casting_fulfilled(const const_entity_handle) const {
	return true;
}

void electric_shield_instance::perform_logic(const spell_logic_input in) {
#if ELECTRIC_SHIELD_AS_SPELL
	const auto subject = in.get_subject();
	const auto& spell_data = std::get<electric_shield>(subject.get_cosmos().get_common_significant().spells);

	ignite_cast_sparkles(spell_data, in.step, subject);
	play_cast_successful_sound(spell_data, in.step, subject);

	in.sentience.get<electric_shield_perk_instance>().timing.set_for_duration(static_cast<float>(spell_data.perk_duration_seconds * 1000), in.now); 
#else
	(void)in;
#endif
}