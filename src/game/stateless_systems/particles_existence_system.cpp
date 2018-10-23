#include "augs/misc/randomization.h"
#include "game/cosmos/logic_step.h"
#include "game/cosmos/cosmos.h"
#include "game/cosmos/entity_handle.h"
#include "game/cosmos/data_living_one_step.h"

#include "game/components/missile_component.h"
#include "game/components/render_component.h"
#include "game/detail/view_input/particle_effect_input.h"
#include "game/components/sentience_component.h"

#include "game/messages/gunshot_message.h"
#include "game/messages/start_particle_effect.h"
#include "game/messages/queue_deletion.h"
#include "game/messages/damage_message.h"
#include "game/messages/melee_swing_response.h"
#include "game/messages/health_event.h"
#include "game/messages/exhausted_cast_message.h"
#include "game/messages/exploding_ring_input.h"

#include "game/stateless_systems/particles_existence_system.h"
#include "game/cosmos/for_each_entity.h"
#include "game/detail/sentience/sentience_getters.h"

void particles_existence_system::displace_streams(const logic_step step) const {
	auto& cosm = step.get_cosmos();
	const auto& clk = cosm.get_clock();

	auto& step_rng = step.step_rng;

	cosm.for_each_having<components::continuous_particles>(
		[&](const auto& it) {
			const auto& cp_def = it.template get<invariants::continuous_particles>();
			const auto& disp = cp_def.displacement;

			if (!disp.is_enabled) {
				return;
			}
			
			const auto& disp_def = disp.value;
			auto& cp = it.template get<components::continuous_particles>();
			auto& state = cp.displacement_state;

			auto& chosen_duration = state.current_duration_ms;

			if (clk.is_ready(chosen_duration, state.when_last)) {
				chosen_duration = step_rng.randval(disp_def.duration_ms);
				state.when_last = clk.now;

				auto& offset = state.current;

				auto h = it.get_logical_size() / 2;

				if (h.x_non_zero()) {
					offset.x = step_rng.randval_h(h.x);
				}

				if (h.y_non_zero()) {
					offset.y = step_rng.randval_h(h.y);
				}

				offset += step_rng.random_point_in_unit_circle<real32>() * disp_def.additional_radius;

				offset.rotate(it.get_logic_transform().rotation, vec2());
			}

			if (DEBUG_DRAWING.enabled) {
				const auto tr = it.get_logic_transform();
				DEBUG_LOGIC_STEP_LINES.emplace_back(white, tr.pos, tr.pos + state.current);
			}
		}
	);
}

void particles_existence_system::play_particles_from_events(const logic_step step) const {
	const auto& gunshots = step.get_queue<messages::gunshot_message>();
	const auto& damages = step.get_queue<messages::damage_message>();
	const auto& healths = step.get_queue<messages::health_event>();
	const auto& exhausted_casts = step.get_queue<messages::exhausted_cast>();
	const auto& logicals = step.get_logical_assets();
	auto& cosm = step.get_cosmos();

	for (const auto& g : gunshots) {
		for (auto& r : g.spawned_rounds) {
			const auto spawned_round = cosm[r];

			{
				const auto& effect = spawned_round.get<invariants::missile>().muzzle_leave_particles;

				effect.start(
					step,
					particle_effect_start_input::orbit_absolute(cosm[g.subject], g.muzzle_transform)
				);
			}

			{
				if (const auto missile = spawned_round.find<invariants::missile>()) {
					const auto& effect = missile->trace_particles;

					const auto rotation = missile->trace_particles_fly_backwards ? 180.f : 0.f;

					effect.start(
						step,
						particle_effect_start_input::orbit_local(cosm[r], { vec2::zero, rotation } )
					);
				}
			}
		}

		const auto shell = cosm[g.spawned_shell];

		if (shell.alive()) {
			const auto& effect = g.cartridge_definition.shell_trace_particles;

			effect.start(
				step,
				particle_effect_start_input::orbit_local(shell, { vec2::zero, 180 } )
			);
		}
	}

	for (const auto& d : damages) {
		const auto subject = cosm[d.subject];

		if (subject.dead()) {
			continue;
		}

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

		{
			auto do_effect = [&](const auto& effect_def) {
				const auto& effect = effect_def.particles;

				{
					effect.start(
						step,
						particle_effect_start_input::orbit_absolute(subject, impact_transform) 
					);
				}

				if (effect_def.spawn_exploding_ring) {
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
			};

			const auto& def = d.effects;

			const bool sentient = sentient_and_vulnerable(subject);

			if (d.inflictor_destructed) {
				do_effect(def.destruction);

				if (sentient) {
					do_effect(def.sentience_impact);
				}
			}
			else {
				if (sentient) {
					do_effect(def.sentience_impact);
				}
				else {
					do_effect(def.impact);
				}
			}
		}

		if (d.type == adverse_element_type::FORCE && d.amount > 0) {
			const auto& fixtures = subject.get<invariants::fixtures>();

			if (const auto* const mat = logicals.find(fixtures.material)) {
				const auto unit = mat->unit_effect_damage;
				const auto mult = d.amount / unit;

				auto effect = mat->standard_damage_particles;

				effect.modifier.scale_amounts *= std::min(4.f, mult);
				effect.modifier.scale_lifetimes *= std::min(1.3f, mult);

				effect.start(
					step,
					particle_effect_start_input::orbit_absolute(subject, impact_transform)
				);
			}
		}
	}

	for (const auto& h : healths) {
		const auto subject = cosm[h.subject];

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
					effect.modifier.scale_amounts = std::min(1.f, h.effective_amount / 100.f);// (1.25f + h.ratio_effective_to_maximum)*(1.25f + h.ratio_effective_to_maximum);
					effect.modifier.scale_lifetimes = 1.25f + std::min(1.f, h.effective_amount / 100.f);
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
		const auto& effect = cosm.get_common_assets().exhausted_smoke_particles;

		effect.start(
			step,
			particle_effect_start_input::at_entity(e.subject)
		);
	}
}