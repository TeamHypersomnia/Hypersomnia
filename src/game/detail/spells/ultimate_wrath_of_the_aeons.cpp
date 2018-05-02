#include "ultimate_wrath_of_the_aeons.h"
#include "game/transcendental/entity_handle.h"
#include "game/transcendental/cosmos.h"
#include "game/components/sentience_component.h"
#include "game/detail/view_input/particle_effect_input.h"
#include "game/detail/view_input/sound_effect_input.h"
#include "game/detail/spells/spell_logic_input.h"
#include "game/detail/spells/spell_utils.h"

bool ultimate_wrath_of_the_aeons_instance::are_additional_conditions_for_casting_fulfilled(const const_entity_handle) const {
	return true;
}

void ultimate_wrath_of_the_aeons_instance::perform_logic(const spell_logic_input in) {
	auto& cosmos = in.step.get_cosmos();
	const auto& spell_data = std::get<ultimate_wrath_of_the_aeons>(cosmos.get_common_significant().spells);
	const auto caster = in.get_subject();
	const auto caster_transform = caster.get_logic_transform();
	const auto step = in.step;
	const auto dt = cosmos.get_fixed_delta();
	const auto now = in.now;
	const auto when_casted = in.when_casted;

	const auto first_at = augs::stepped_timestamp{ when_casted.step + static_cast<unsigned>(1.3f / dt.in_seconds()) };
	const auto second_at = augs::stepped_timestamp{ when_casted.step + static_cast<unsigned>(1.8f / dt.in_seconds()) };
	const auto third_at = augs::stepped_timestamp{ when_casted.step + static_cast<unsigned>(2.3f / dt.in_seconds()) };

	if (now == when_casted) {
		play_cast_successful_sound(spell_data, in.step, caster);
		ignite_cast_sparkles(spell_data, in.step, caster);
		ignite_charging_particles(spell_data, in.step, caster, cyan);
		ignite_charging_particles(spell_data, in.step, caster, white);
		play_cast_charging_sound(spell_data, in.step, caster);
	}
	else if (now == first_at) {
		spell_data.explosions[0].instantiate(step, caster_transform, caster);
	}
	else if (now == second_at) {
		spell_data.explosions[1].instantiate(step, caster_transform, caster);
	}
	else if (now == third_at) {
		spell_data.explosions[2].instantiate(step, caster_transform, caster);
	}
}