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
		filters[predefined_filter_type::FLYING_ITEM]
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
		filters[predefined_filter_type::FLYING_ITEM]
	);

	if (hostiles.empty()) {
		return;
	}

	const auto num_enemies = hostiles.size();

	for (std::size_t i = 0; i < 3; ++i) {
		const auto next_hostile = cosm[hostiles[std::min(i, num_enemies - 1)]];
#if MORE_LOGS
		LOG_NVPS(next_hostile.get_id());
#endif

		cosmic::create_entity(
			cosm, 
			spell_data.missile_flavour,
			[&](const auto new_energy_ball, auto&&...) {
				const auto target_vector = next_hostile.get_logic_transform().pos - caster_transform.pos;
				const auto target_degrees = target_vector.degrees();

				auto new_energy_ball_transform = caster_transform;

				auto& rot = new_energy_ball_transform.rotation;

				{
					const bool lacking_hostiles = num_enemies < i + 1;

					if (lacking_hostiles) {
						if (num_enemies == 1) {
							const auto spread = spell_data.spread_in_absence_of_hostiles;

							rot = target_degrees;

							if (i == 1) {
								rot += spread / 2;
							}
							else {
								rot -= spread / 2;
							}
						}
						else if (num_enemies == 2) {
							const auto prev_hostile = cosm[hostiles[0]];
							const auto prev_target_vector = prev_hostile.get_logic_transform().pos - caster_transform.pos;
							const auto avg_vector = (prev_target_vector + target_vector) / 2;

							rot = avg_vector.degrees();
						}
					}	
					else {
						rot = target_degrees;
					}
				}

				new_energy_ball.set_logic_transform(new_energy_ball_transform);

				new_energy_ball.template get<components::sender>().set(caster);
				new_energy_ball.template get<components::missile>().particular_homing_target = next_hostile;

				const auto energy_ball_velocity = new_energy_ball_transform.get_direction() * spell_data.missile_velocity;
				new_energy_ball.template get<components::rigid_body>().set_velocity(energy_ball_velocity);
			},
			[&](const auto) {}
		);
	}
}