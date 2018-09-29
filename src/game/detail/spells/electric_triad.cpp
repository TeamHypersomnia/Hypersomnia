#include "game/detail/spells/electric_triad.h"
#include "game/cosmos/entity_handle.h"
#include "game/cosmos/cosmos.h"
#include "game/components/sentience_component.h"
#include "game/detail/view_input/particle_effect_input.h"
#include "game/detail/view_input/sound_effect_input.h"
#include "game/components/sender_component.h"
#include "game/components/missile_component.h"
#include "game/detail/spells/spell_logic_input.h"
#include "game/detail/spells/spell_utils.h"
#include "game/enums/filters.h"
#include "game/cosmos/create_entity.hpp"

bool electric_triad_instance::are_additional_conditions_for_casting_fulfilled(const const_entity_handle caster) const {
	constexpr float standard_triad_radius = 800.f;

	const bool is_any_hostile_in_proximity = get_closest_hostiles(
		caster,
		caster,
		standard_triad_radius,
		filters::flying_item()
	).size() > 0;

	return is_any_hostile_in_proximity;
}

void electric_triad_instance::perform_logic(const spell_logic_input in) {
	const auto subject = in.get_subject();
	const auto& spell_data = std::get<electric_triad>(subject.get_cosmos().get_common_significant().spells);
	const auto caster_transform = subject.get_logic_transform();

	ignite_cast_sparkles(spell_data, in.step, subject);
	play_cast_successful_sound(spell_data, in.step, subject);

	constexpr float standard_triad_radius = 800.f;
	const auto caster = subject;
	auto& cosm = caster.get_cosmos();

	if (!spell_data.missile_flavour.is_set()) {
		return;
	}
	
	const auto hostiles = get_closest_hostiles(
		caster,
		caster,
		standard_triad_radius,
		filters::flying_item()
	);

	for (unsigned i = 0; i < 3 && i < static_cast<unsigned>(hostiles.size()); ++i) {
		const auto next_hostile = cosm[hostiles[i]];
#if MORE_LOGS
		LOG_NVPS(next_hostile.get_id());
#endif

		cosmic::create_entity(
			cosm, 
			spell_data.missile_flavour,
			[&](const auto new_energy_ball, auto&&...) {
				auto new_energy_ball_transform = caster_transform;

				new_energy_ball_transform.rotation = 
					(next_hostile.get_logic_transform().pos - caster_transform.pos).degrees()
				;

				new_energy_ball.set_logic_transform(new_energy_ball_transform);

				new_energy_ball.template get<components::sender>().set(caster);
				new_energy_ball.template get<components::missile>().particular_homing_target = next_hostile;

				const auto energy_ball_velocity = vec2::from_degrees(new_energy_ball_transform.rotation) * 2000;
				new_energy_ball.template get<components::rigid_body>().set_velocity(energy_ball_velocity);
			},
			[&](const auto) {}
		);
	}
}