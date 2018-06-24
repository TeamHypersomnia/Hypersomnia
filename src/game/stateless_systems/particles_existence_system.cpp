#include "augs/misc/randomization.h"
#include "game/transcendental/logic_step.h"
#include "game/transcendental/cosmos.h"
#include "game/transcendental/data_living_one_step.h"

#include "game/components/missile_component.h"
#include "game/components/render_component.h"
#include "game/detail/view_input/particle_effect_input.h"
#include "game/components/sentience_component.h"

#include "game/messages/gunshot_message.h"
#include "game/messages/start_particle_effect.h"
#include "game/messages/queue_destruction.h"
#include "game/messages/damage_message.h"
#include "game/messages/melee_swing_response.h"
#include "game/messages/health_event.h"
#include "game/messages/exhausted_cast_message.h"
#include "game/messages/exploding_ring_input.h"

#include "game/stateless_systems/particles_existence_system.h"

void particles_existence_system::game_responses_to_particle_effects(const logic_step step) const {
	const auto& gunshots = step.get_queue<messages::gunshot_message>();
	const auto& damages = step.get_queue<messages::damage_message>();
	const auto& healths = step.get_queue<messages::health_event>();
	const auto& exhausted_casts = step.get_queue<messages::exhausted_cast>();
	const auto& logicals = step.get_logical_assets();
	auto& cosmos = step.get_cosmos();

	for (const auto& g : gunshots) {
		for (auto& r : g.spawned_rounds) {
			const auto spawned_round = cosmos[r];

			{
				const auto& effect = spawned_round.get<invariants::missile>().muzzle_leave_particles;

				effect.start(
					step,
					particle_effect_start_input::orbit_absolute(cosmos[g.subject], g.muzzle_transform)
				);
			}

			{
				if (const auto missile = spawned_round.find<invariants::missile>()) {
					const auto& effect = missile->trace_particles;

					const auto rotation = missile->trace_particles_fly_backwards ? 180.f : 0.f;

					effect.start(
						step,
						particle_effect_start_input::orbit_local(cosmos[r], { vec2::zero, rotation } )
					);
				}
			}
		}

		const auto shell = cosmos[g.spawned_shell];

		if (shell.alive()) {
			const auto& effect = g.cartridge_definition.shell_trace_particles;

			effect.start(
				step,
				particle_effect_start_input::orbit_local(shell, { vec2::zero, 180 } )
			);
		}
	}

	for (const auto& d : damages) {
		const auto impact_transform = [&]() {
			auto result = transformr(d.point_of_impact);			

			if (d.amount > 0) {
				result.rotation = (-d.impact_velocity).degrees();
			}
			else {
				result.rotation = (d.impact_velocity).degrees();
			}

			return result;
		}();

		if (d.inflictor_destructed) {
			const auto inflictor = cosmos[d.inflictor];

			const auto& effect = inflictor.get<invariants::missile>().destruction_particles;

			{
				effect.start(
					step,
					particle_effect_start_input::orbit_absolute(cosmos[d.subject], transformr{impact_transform.pos, 0} ) 
				);
			}

			{
				const auto max_radius = d.amount * 1.5f;

				exploding_ring_input ring;

				ring.outer_radius_start_value = max_radius;
				ring.outer_radius_end_value = max_radius / 1.2f;

				ring.inner_radius_start_value = max_radius / 1.4f;
				ring.inner_radius_end_value = max_radius / 1.2f;

				ring.emit_particles_on_ring = false;

				ring.maximum_duration_seconds = 0.16f;

				ring.color = effect.modifier.colorize;
				ring.center = impact_transform.pos;

				step.post_message(ring);
			}
		}

		if (d.type == adverse_element_type::FORCE && d.amount > 0) {
			if (const auto subject = cosmos[d.subject]; subject.alive()) {
				const auto& fixtures = subject.get<invariants::fixtures>();

				if (const auto* const mat = logicals.find(fixtures.material)) {
					const auto unit = mat->unit_effect_damage;
					const auto mult = d.amount / unit;

					auto effect = mat->standard_damage_particles;

					effect.modifier.scale_amounts *= mult;
					effect.modifier.scale_lifetimes *= std::min(1.3f, mult);

					effect.start(
						step,
						particle_effect_start_input::orbit_absolute(cosmos[d.subject], impact_transform)
					);
				}
			}
		}
	}

	for (const auto& h : healths) {
		const auto subject = cosmos[h.subject];

		if (h.target == messages::health_event::target_type::HEALTH) {
			const auto& sentience = subject.get<invariants::sentience>();

			if (h.effective_amount > 0) {
				auto effect = sentience.health_decrease_particles;

				if (h.special_result == messages::health_event::result_type::DEATH) {
					effect.modifier.scale_amounts = 0.6f;
					effect.modifier.scale_lifetimes = 1.25f;
					effect.modifier.colorize = red;
				}
				else {
					effect.modifier.colorize = red;

					/* Fix it when we can specify modifiers per-emission */
					effect.modifier.scale_amounts = h.effective_amount / 100.f;// (1.25f + h.ratio_effective_to_maximum)*(1.25f + h.ratio_effective_to_maximum);
					effect.modifier.scale_lifetimes = 1.25f + h.effective_amount / 100.f;
				}

				messages::stop_particle_effect stop;
				stop.match_chased_subject = h.subject;
				stop.match_orbit_offset = vec2::zero;
				stop.match_effect_id = effect.id;
				step.post_message(stop);

				auto start_in = particle_effect_start_input::orbit_local(h.subject, { vec2::zero, (h.impact_velocity).degrees() });
				start_in.homing_target = h.subject;

				effect.start(
					step,
					start_in
				);
			}
		}
	}

	for (const auto& e : exhausted_casts) {
		const auto& effect = cosmos.get_common_assets().exhausted_smoke_particles;

		effect.start(
			step,
			particle_effect_start_input::at_entity(e.subject)
		);
	}
}