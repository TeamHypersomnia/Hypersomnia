#include "particles_existence_system.h"
#include "game/transcendental/logic_step.h"
#include "game/transcendental/cosmos.h"
#include "game/components/particles_existence_component.h"
#include "game/messages/queue_destruction.h"

#include "game/assets/particle_effect.h"

#include "game/components/damage_component.h"
#include "game/components/render_component.h"
#include "game/components/position_copying_component.h"
#include "game/components/particles_existence_component.h"
#include "game/components/sentience_component.h"

#include "game/messages/gunshot_response.h"
#include "game/messages/create_particle_effect.h"
#include "game/messages/queue_destruction.h"
#include "game/messages/damage_message.h"
#include "game/messages/melee_swing_response.h"
#include "game/messages/health_event.h"
#include "game/messages/exhausted_cast_message.h"

#include "game/systems_audiovisual/particles_simulation_system.h"
#include "game/detail/particle_types.h"

bool components::particles_existence::is_activated(const const_entity_handle h) {
	return h.get<components::tree_of_npo_node>().is_activated() && h.get<components::processing>().is_in(processing_subjects::WITH_PARTICLES_EXISTENCE);
}

void components::particles_existence::activate(const entity_handle h) {
	if (is_activated(h)) {
		return;
	}

	auto& existence = h.get<components::particles_existence>();
	existence.time_of_birth = h.get_cosmos().get_timestamp();
	h.get<components::tree_of_npo_node>().set_activated(true);
	h.get<components::processing>().enable_in(processing_subjects::WITH_PARTICLES_EXISTENCE);
}

void components::particles_existence::deactivate(const entity_handle h) {
	if (!is_activated(h)) {
		return;
	}

	h.get<components::tree_of_npo_node>().set_activated(false);
	h.get<components::processing>().disable_in(processing_subjects::WITH_PARTICLES_EXISTENCE);
}

void particles_existence_system::displace_streams_and_destroy_dead_streams(const logic_step step) const {
	auto& cosmos = step.cosm;
	const auto timestamp = cosmos.get_timestamp();

	cosmos.for_each(
		processing_subjects::WITH_PARTICLES_EXISTENCE,
		[&](const auto it) {
			auto& existence = it.get<components::particles_existence>();
			auto& input = existence.input;

			if (input.displace_source_position_within_radius > 0.f) {
				if ((timestamp - existence.time_of_last_displacement).in_milliseconds(step.get_delta()) > existence.current_displacement_duration_bound_ms) {
					const auto new_seed = cosmos.get_rng_seed_for(it) + cosmos.get_total_steps_passed();
					randomization rng(new_seed);

					existence.time_of_last_displacement = timestamp;
					existence.current_displacement.set_from_degrees(rng.randval(0.f, 360.f)).set_length(rng.randval(0.f, input.displace_source_position_within_radius));
					existence.current_displacement_duration_bound_ms = rng.randval(input.single_displacement_duration_ms);
				}
			}

			if ((timestamp - existence.time_of_birth).step > existence.max_lifetime_in_steps) {
				if (existence.input.delete_entity_after_effect_lifetime) {
					step.transient.messages.post(messages::queue_destruction(it));
				}
				else {
					components::particles_existence::deactivate(it);
				}
			}
		},
		{ subjects_iteration_flag::POSSIBLE_ITERATOR_INVALIDATION }
	);
}

void particles_existence_system::game_responses_to_particle_effects(const logic_step step) const {
	const auto& gunshots = step.transient.messages.get_queue<messages::gunshot_response>();
	const auto& damages = step.transient.messages.get_queue<messages::damage_message>();
	const auto& swings = step.transient.messages.get_queue<messages::melee_swing_response>();
	const auto& healths = step.transient.messages.get_queue<messages::health_event>();
	const auto& exhausted_casts = step.transient.messages.get_queue<messages::exhausted_cast>();
	auto& cosmos = step.cosm;

	for (const auto& g : gunshots) {
		for (auto& r : g.spawned_rounds) {
			{
				particle_effect_input burst;
				burst.effect = cosmos[r].get<components::damage>().muzzle_leave_particle_effect_response;

				burst.create_particle_effect_entity(
					step,
					g.muzzle_transform,
					g.subject
				).add_standard_components(step);
			}

			{
				particle_effect_input particle_trace;
				particle_trace.effect = cosmos[r].get<components::damage>().bullet_trace_particle_effect_response;

				auto place_of_birth = cosmos[r].get_logic_transform();
				place_of_birth.rotation += 180;

				particle_trace.create_particle_effect_entity(
					step,
					place_of_birth,
					r
				).add_standard_components(step);
			}
		}

		const auto shell = cosmos[g.spawned_shell];

		if (shell.alive()) {
			particle_effect_input burst;
			burst.effect = g.catridge_definition.shell_trace_particle_effect_response;

			auto place_of_birth = shell.get_logic_transform();
			place_of_birth.rotation += 180;

			burst.create_particle_effect_entity(
				step,
				place_of_birth,
				shell
			).add_standard_components(step);
		}
	}

	for (const auto& d : damages) {
		if (d.inflictor_destructed) {
			const auto inflictor = cosmos[d.inflictor];

			particle_effect_input burst;
			
			auto place_of_birth = components::transform(d.point_of_impact);

			if (d.amount > 0) {
				place_of_birth.rotation = (-d.impact_velocity).degrees();
			}
			else {
				place_of_birth.rotation = (d.impact_velocity).degrees();
			}

			burst.effect = inflictor.get<components::damage>().destruction_particle_effect_response;

			burst.create_particle_effect_entity(
				step,
				place_of_birth,
				d.subject
			).add_standard_components(step);
		}
	}

	for (const auto& h : healths) {
		const auto subject = cosmos[h.subject];
		const auto& sentience = subject.get<components::sentience>();
		const auto particles_entity = cosmos[sentience.health_damage_particles];
		auto& existence = particles_entity.get<components::particles_existence>();

		particle_effect_input burst = existence.input;

		auto place_of_birth = components::transform(h.point_of_impact);

		place_of_birth.pos = cosmos[h.subject].get_logic_transform().pos;
		place_of_birth.rotation = (h.impact_velocity).degrees();
		
		if (h.target == messages::health_event::target_type::HEALTH) {
			const auto& sentience = subject.get<components::sentience>();

			if (h.effective_amount > 0) {
				burst.effect = sentience.health_decrease_particle_effect_response;

				if (h.special_result == messages::health_event::result_type::DEATH) {
					burst.effect.modifier.scale_amounts = 0.6f;
					burst.effect.modifier.scale_lifetimes = 1.25f;
					burst.effect.modifier.colorize = red;
					burst.effect.modifier.homing_target = h.subject;
				}
				else {
					burst.effect.modifier.colorize = red;
					burst.effect.modifier.scale_amounts = h.effective_amount / 100.f;// (1.25f + h.ratio_effective_to_maximum)*(1.25f + h.ratio_effective_to_maximum);
					burst.effect.modifier.scale_lifetimes = 1.25f + h.effective_amount / 100.f;
					burst.effect.modifier.homing_target = h.subject;
				}

				burst.create_particle_effect_components(
					particles_entity.get<components::transform>(),
					existence,
					particles_entity.get<components::position_copying>(),
					step,
					place_of_birth,
					h.subject
				);
			}
			
			components::particles_existence::activate(particles_entity);
		}
	}

	for (const auto& e : exhausted_casts) {
		particle_effect_input burst;
		burst.effect.id = assets::particle_effect_id::EXHAUSTED_SMOKE;

		burst.create_particle_effect_entity(
			step,
			e.transform,
			e.subject
		).add_standard_components(step);
	}
}