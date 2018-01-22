#include "electric_triad.h"
#include "game/transcendental/entity_handle.h"
#include "game/transcendental/cosmos.h"
#include "game/components/sentience_component.h"
#include "game/components/particles_existence_component.h"
#include "game/components/sound_existence_component.h"
#include "game/components/sender_component.h"
#include "game/components/missile_component.h"
#include "game/components/all_inferred_state_component.h"
#include "game/detail/spells/spell_logic_input.h"
#include "game/detail/spells/spell_utils.h"
#include "game/enums/filters.h"

bool electric_triad_instance::are_additional_conditions_for_casting_fulfilled(const const_entity_handle caster) const {
	constexpr float standard_triad_radius = 800.f;

	const bool is_any_hostile_in_proximity = get_closest_hostiles(
		caster,
		caster,
		standard_triad_radius,
		filters::bullet()
	).size() > 0;

	return is_any_hostile_in_proximity;
}

void electric_triad_instance::perform_logic(const spell_logic_input in) {
	const auto& spell_data = std::get<electric_triad>(in.subject.get_cosmos().get_common_significant().spells);
	const auto caster_transform = in.subject.get_logic_transform();

	ignite_cast_sparkles(spell_data, in.step, caster_transform, in.subject);
	play_cast_successful_sound(spell_data, in.step, caster_transform, in.subject);

	constexpr float standard_triad_radius = 800.f;
	const auto caster = in.subject;
	auto& cosmos = caster.get_cosmos();

	if (cosmos[spell_data.missile_definition].dead()) {
		return;
	}
	
	const auto hostiles = get_closest_hostiles(
		caster,
		caster,
		standard_triad_radius,
		filters::bullet()
	);

	for (unsigned i = 0; i < 3 && i < static_cast<unsigned>(hostiles.size()); ++i) {
		const auto next_hostile = cosmos[hostiles[i]];
		LOG_NVPS(next_hostile.get_id());

		const auto new_energy_ball = cosmic::clone_entity(cosmos[spell_data.missile_definition]);

		auto new_energy_ball_transform = caster_transform;
		
		new_energy_ball_transform.rotation = 
			(next_hostile.get_logic_transform().pos - caster_transform.pos).degrees();
		
		new_energy_ball.set_logic_transform(in.step, new_energy_ball_transform);

		new_energy_ball.get<components::sender>().set(caster);
		new_energy_ball.get<components::missile>().particular_homing_target = next_hostile;

		const auto energy_ball_velocity = vec2::from_degrees(new_energy_ball_transform.rotation) * 2000;
		new_energy_ball.get<components::rigid_body>().set_velocity(energy_ball_velocity);

		new_energy_ball.add_standard_components(in.step);
	}
}