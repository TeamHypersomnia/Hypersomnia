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
#include "game/cosmos/data_living_one_step.h"

bool electric_triad_instance::are_additional_conditions_for_casting_fulfilled(const const_entity_handle caster) const {
	(void)caster;
	return true;
}

void electric_triad_instance::perform_logic(const spell_logic_input in) {
	auto& cosm = in.step.get_cosmos();
	const auto caster = in.get_subject();
	const auto& spell_data = std::get<electric_triad>(caster.get_cosmos().get_common_significant().spells);
	const auto caster_transform = caster.get_logic_transform();

	const auto clk = cosm.get_clock();	
	const auto now = clk.now;
	const auto dt = clk.dt;
	const auto when_casted = in.when_casted;

	const auto first_at = augs::stepped_timestamp{ when_casted.step + static_cast<unsigned>(0.8f / dt.in_seconds()) };
	const auto second_at = augs::stepped_timestamp{ when_casted.step + static_cast<unsigned>(1.0f / dt.in_seconds()) };
	const auto third_at = augs::stepped_timestamp{ when_casted.step + static_cast<unsigned>(1.2f / dt.in_seconds()) };

	if (now == when_casted) {
		const auto& charging_data = std::get<ultimate_wrath_of_the_aeons>(cosm.get_common_significant().spells);

		play_cast_successful_sound(spell_data, in.step, caster);
		ignite_cast_sparkles(spell_data, in.step, caster);
		ignite_charging_particles(charging_data, in.step, caster, cyan);
		ignite_charging_particles(charging_data, in.step, caster, white);
		play_cast_charging_sound(charging_data, in.step, caster);
		return;
	}

	auto create_nth = [&](const auto i) {
		ignite_cast_sparkles(spell_data, in.step, caster);
		play_cast_successful_sound(spell_data, in.step, caster);

		if (!spell_data.missile_flavour.is_set()) {
			return;
		}
		
		const auto spread = spell_data.spread_in_absence_of_hostiles;

		auto missile_velocity = spell_data.missile_velocity;

		components::sender sender_info;
		sender_info.set(caster);

		{
			cosmic::queue_create_entity(
				in.step, 
				spell_data.missile_flavour,
				[i, spread, missile_velocity, caster_transform, sender_info](const entity_handle new_energy_ball, auto&) {
					auto new_energy_ball_transform = caster_transform;
					auto& rot = new_energy_ball_transform.rotation;

					if (i == 1) {
						rot += spread / 2;
					}
					else if (i == 2) {
						rot -= spread / 2;
					}

					new_energy_ball.set_logic_transform(new_energy_ball_transform);

					new_energy_ball.template get<components::sender>() = sender_info;

					const auto energy_ball_velocity = new_energy_ball_transform.get_direction() * missile_velocity;
					new_energy_ball.template get<components::rigid_body>().set_velocity(energy_ball_velocity);
				}
			);
		}
	};

	if (now == first_at) {
		create_nth(0);
	}

	if (now == second_at) {
		create_nth(1);
	}

	if (now == third_at) {
		create_nth(2);
	}
}