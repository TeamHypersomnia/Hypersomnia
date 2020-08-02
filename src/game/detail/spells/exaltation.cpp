#include "exaltation.h"
#include "game/cosmos/entity_handle.h"
#include "game/cosmos/cosmos.h"
#include "game/components/sentience_component.h"
#include "game/detail/view_input/particle_effect_input.h"
#include "game/detail/view_input/sound_effect_input.h"
#include "game/detail/spells/spell_logic_input.h"
#include "game/detail/spells/spell_utils.h"
#include "game/stateless_systems/sentience_system.h"

#include "game/messages/health_event.h"

bool exaltation_instance::are_additional_conditions_for_casting_fulfilled(const const_entity_handle subject) const {
	const auto& spell_data = std::get<exaltation>(subject.get_cosmos().get_common_significant().spells);

	auto& health = subject.get<components::sentience>().get<health_meter_instance>();
	
	const bool would_heal_anything =
		health.calc_damage_result(-spell_data.basic_healing_amount).effective < 0
	;

	return would_heal_anything;
}

void exaltation_instance::perform_logic(const spell_logic_input in) {
	const auto subject = in.get_subject();
	const auto& spell_data = std::get<exaltation>(subject.get_cosmos().get_common_significant().spells);

	ignite_cast_sparkles(spell_data, in.step, subject);
	play_cast_successful_sound(spell_data, in.step, subject);

	auto& health = in.sentience.get<health_meter_instance>();

	messages::health_event event;
	event.subject = subject;
	event.point_of_impact = subject.get_logic_transform().pos;
	event.impact_velocity = { 0, -200 };
	event.damage = health.calc_damage_result(-spell_data.basic_healing_amount);
	event.target = messages::health_event::target_type::HEALTH;

	sentience_system().process_and_post_health_event(event, in.step);
}