#include "echoes_of_the_higher_realms.h"
#include "game/transcendental/entity_handle.h"
#include "game/transcendental/cosmos.h"
#include "game/components/sentience_component.h"
#include "game/components/particles_existence_component.h"
#include "game/components/sound_existence_component.h"
#include "game/detail/spells/spell_logic_input.h"
#include "game/detail/spells/spell_utils.h"
#include "game/systems_stateless/sentience_system.h"

#include "game/messages/health_event.h"

bool echoes_of_the_higher_realms_instance::are_additional_conditions_for_casting_fulfilled(const const_entity_handle subject) const {
	const auto& spell_data = std::get<echoes_of_the_higher_realms>(subject.get_cosmos().get_global_state().spells);

	auto& consciousness = subject.get<components::sentience>().get<consciousness_meter_instance>();
	
	const bool would_heal_anything =
		consciousness.calculate_damage_result(-spell_data.basic_healing_amount).effective < 0
	;

	return would_heal_anything;
}

void echoes_of_the_higher_realms_instance::perform_logic(const spell_logic_input in) {
	const auto& spell_data = std::get<echoes_of_the_higher_realms>(in.subject.get_cosmos().get_global_state().spells);
	const auto caster_transform = in.subject.get_logic_transform();

	ignite_cast_sparkles(spell_data, in.step, caster_transform, in.subject);
	play_cast_successful_sound(spell_data, in.step, caster_transform, in.subject);

	auto& consciousness = in.sentience.get<consciousness_meter_instance>();

	const auto result = consciousness.calculate_damage_result(-spell_data.basic_healing_amount);

	messages::health_event event;
	event.subject = in.subject;
	event.point_of_impact = in.subject.get_logic_transform().pos;
	event.impact_velocity = { 0, -200 };
	event.effective_amount = result.effective;
	event.target = messages::health_event::target_type::CONSCIOUSNESS;
	event.ratio_effective_to_maximum = result.ratio_effective_to_maximum;

	sentience_system().consume_health_event(event, in.step);
}