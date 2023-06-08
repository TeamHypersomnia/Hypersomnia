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
#include "game/messages/health_event.h"
#include "game/messages/exhausted_cast_message.h"
#include "game/messages/exploding_ring_effect.h"
#include "game/messages/exploding_ring_effect.h"
#include "game/messages/thunder_effect.h"

#include "game/stateless_systems/particles_existence_system.h"
#include "game/cosmos/for_each_entity.h"
#include "game/detail/sentience/sentience_getters.h"
#include "game/detail/physics/calc_physical_material.hpp"

void particles_existence_system::displace_streams(const logic_step step) const {
	auto& cosm = step.get_cosmos();
	const auto& clk = cosm.get_clock();

	auto& step_rng = step.step_rng;

	cosm.for_each_having<components::continuous_particles>(
		[&](const auto& it) {
			const auto& cp_def = it.template get<invariants::continuous_particles>();
			const auto& disp = cp_def.wandering;

			if (!disp.is_enabled) {
				return;
			}
			
			const auto& disp_def = disp.value;
			auto& cp = it.template get<components::continuous_particles>();
			auto& state = cp.wandering_state;

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
		const auto predictability = predictable_only_by(g.capability);

		for (auto& r : g.spawned_rounds) {
			const auto spawned_round = cosm[r];

			{
				const auto& effect = spawned_round.get<invariants::missile>().muzzle_leave_particles;

				effect.start(
					step,
					particle_effect_start_input::orbit_absolute(cosm[g.subject], g.muzzle_transform),
					predictability
				);
			}

			{
				if (const auto missile = spawned_round.find<invariants::missile>()) {
					const auto& effect = missile->trace_particles;

					const auto rotation = missile->trace_particles_fly_backwards ? 180.f : 0.f;

					effect.start(
						step,
						particle_effect_start_input::orbit_local(cosm[r], { vec2::zero, rotation } ),
						predictability
					);
				}
			}
		}
	}

	for (const auto& d : damages) {
		const auto subject = cosm[d.subject];

		if (subject.dead()) {
			continue;
		}

		const auto impact_transform = [&]() {
			auto result = transformr(d.point_of_impact);			

			if (d.damage.base > 0) {
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
						particle_effect_start_input::orbit_absolute(subject, impact_transform),
						always_predictable_v
					);
				}

				if (effect_def.spawn_exploding_ring) {
					const auto max_radius = d.damage.base * 1.5f;

					auto msg = messages::exploding_ring_effect(always_predictable_v);
					auto& ring = msg.payload;

					ring.outer_radius_start_value = max_radius;
					ring.outer_radius_end_value = max_radius / 1.2f;

					ring.inner_radius_start_value = max_radius / 1.4f;
					ring.inner_radius_end_value = max_radius / 1.2f;

					ring.emit_particles_on_ring = false;

					ring.maximum_duration_seconds = 0.16f;

					ring.color = effect.modifier.color;
					ring.center = impact_transform.pos;

					step.post_message(std::move(msg));
				}
			};

			const auto& e = d.damage.effects;

			const bool sentient = subject.template has<components::sentience>();
			const bool vulnerable = sentient_and_vulnerable(subject);

			if (d.spawn_destruction_effects) {
				if (sentient) {
					if (e.sentience_impact.particles.id.is_set()) {
						do_effect(e.sentience_impact);
					}
				}
				else {
					do_effect(e.destruction);
				}
			}
			else {
				if (vulnerable && e.sentience_impact.particles.id.is_set()) {
					do_effect(e.sentience_impact);
				}
				else {
					do_effect(e.impact);
				}
			}
		}

		if (d.type == adverse_element_type::FORCE && d.damage.base > 0) {
			if (const auto* const mat = logicals.find(calc_physical_material(subject))) {
				const auto unit = mat->unit_damage_for_effects;
				const auto mult = d.damage.base / unit;

				auto effect = mat->standard_damage_particles;

				effect.modifier.scale_amounts *= std::min(4.f, mult);
				effect.modifier.scale_lifetimes *= std::min(1.3f, mult);

				const auto start = particle_effect_start_input::orbit_absolute(subject, impact_transform);

				effect.start(
					step,
					start,
					always_predictable_v
				);
			}
		}
	}

	for (const auto& h : healths) {
		const auto subject = cosm[h.subject];

		const bool destroyed = h.special_result != messages::health_event::result_type::NONE;

		auto predictability = destroyed ? never_predictable_v : always_predictable_v;

		if (step.get_settings().effect_prediction.predict_death_particles) {
			predictability = always_predictable_v;
		}

		if (h.target == messages::health_event::target_type::HEALTH) {
			const auto& sentience = subject.get<invariants::sentience>();

			if (h.damage.total() > 0) {
				auto effect = sentience.health_decrease_particles;

				effect.modifier.scale_amounts *= std::min(1.f, h.damage.total() / 100.f);// (1.25f + h.damage.ratio_effective_to_maximum)*(1.25f + h.damage.ratio_effective_to_maximum);
				//effect.modifier.scale_lifetimes *= std::min(1.f, h.damage.total() / 100.f);

				{
					auto stop = messages::stop_particle_effect(predictability);
					stop.match_chased_subject = h.subject;
					stop.match_orbit_offset = vec2::zero;
					stop.match_effect_id = effect.id;
					step.post_message(stop);
				}

				auto start_in = particle_effect_start_input::orbit_absolute(cosm[h.subject], { h.point_of_impact, (-h.impact_velocity).degrees() });
				start_in.homing_target = h.subject;

				effect.start(
					step,
					start_in,
					predictability
				);
			}
		}

		auto make_ring_input = [predictability]() {
			return messages::exploding_ring_effect(predictability);
		};

		auto make_thunder_input = [predictability]() {
			return messages::thunder_effect(predictability);
		};

		if (h.target == messages::health_event::target_type::HEALTH) {
			if (h.damage.total() > 0) {
				const auto base_radius = destroyed ? 80.f : h.damage.total() * 1.5f;

				{
					auto msg = make_ring_input();
					auto& ring = msg.payload;

					ring.outer_radius_start_value = base_radius / 1.5f;
					ring.outer_radius_end_value = base_radius / 3.f;

					ring.inner_radius_start_value = base_radius / 2.5f;
					ring.inner_radius_end_value = base_radius / 3.f;

					ring.emit_particles_on_ring = false;

					ring.maximum_duration_seconds = 0.20f;

					ring.color = white;
					ring.center = h.point_of_impact;

					//step.post_message(std::move(msg));
				}

				{
					auto msg = make_ring_input();
					auto& ring = msg.payload;

					ring.outer_radius_start_value = base_radius / 2.f;
					ring.outer_radius_end_value = base_radius;

					ring.inner_radius_start_value = 0.f;
					ring.inner_radius_end_value = base_radius;

					ring.emit_particles_on_ring = false;

					ring.maximum_duration_seconds = 0.20f;

					ring.color = white;
					ring.center = h.point_of_impact;
					
					//step.post_message(std::move(msg));
				}

				{
					auto msg = make_thunder_input();
					auto& th = msg.payload;

					th.delay_between_branches_ms = { 8.f, 17.f };
					th.max_branch_lifetime_ms = { 50.f, 80.f };
					th.branch_length = { 40.f, 70.f };

					th.max_all_spawned_branches = static_cast<unsigned>(h.damage.total());
					++th.max_all_spawned_branches;
					th.max_branch_children = 4;

					th.first_branch_root = h.point_of_impact;
					th.first_branch_root.rotation = (-h.impact_velocity).degrees();
					th.branch_angle_spread = 45.f;

					th.color = white;

					step.post_message(std::move(msg));
				}
			}
		}
		else if (h.target == messages::health_event::target_type::PERSONAL_ELECTRICITY) {
			if (h.damage.total() > 0) {
				const auto base_radius = destroyed ? 250.f : h.damage.total() * 1.2f;

				{
					auto msg = make_ring_input();
					auto& ring = msg.payload;

					ring.outer_radius_start_value = base_radius / 1.5f;
					ring.outer_radius_end_value = base_radius / 3.f;

					ring.inner_radius_start_value = base_radius / 2.5f;
					ring.inner_radius_end_value = base_radius / 3.f;

					ring.emit_particles_on_ring = false;

					ring.maximum_duration_seconds = 0.20f;

					ring.color = cyan;
					ring.center = h.point_of_impact;

					step.post_message(std::move(msg));
				}

				{
					auto msg = make_ring_input();
					auto& ring = msg.payload;

					ring.outer_radius_start_value = base_radius / 2;
					ring.outer_radius_end_value = base_radius;

					ring.inner_radius_start_value = 0.f;
					ring.inner_radius_end_value = base_radius;

					ring.emit_particles_on_ring = false;

					ring.maximum_duration_seconds = 0.20f;

					ring.color = turquoise;
					ring.center = h.point_of_impact;

					step.post_message(std::move(msg));
				}
			}
		}
		else if (h.target == messages::health_event::target_type::CONSCIOUSNESS) {
			if (h.damage.total() > 0) {
				const auto base_radius = destroyed ? 80.f : h.damage.total() * 2.f;
				{
					auto msg = make_ring_input();
					auto& ring = msg.payload;

					ring.outer_radius_start_value = base_radius / 1.5f;
					ring.outer_radius_end_value = base_radius / 3.f;

					ring.inner_radius_start_value = base_radius / 2.5f;
					ring.inner_radius_end_value = base_radius / 3.f;

					ring.emit_particles_on_ring = false;

					ring.maximum_duration_seconds = 0.20f;

					ring.color = yellow;
					ring.center = h.point_of_impact;

					step.post_message(std::move(msg));
				}

				{
					auto msg = make_ring_input();
					auto& ring = msg.payload;

					ring.outer_radius_start_value = base_radius / 2.f;
					ring.outer_radius_end_value = base_radius;

					ring.inner_radius_start_value = 0.f;
					ring.inner_radius_end_value = base_radius;

					ring.emit_particles_on_ring = false;

					ring.maximum_duration_seconds = 0.20f;

					ring.color = orange;
					ring.center = h.point_of_impact;

					step.post_message(std::move(msg));
				}
			}
		}
	}

	for (const auto& e : exhausted_casts) {
		const auto& effect = cosm.get_common_assets().exhausted_smoke_particles;

		effect.start(
			step,
			particle_effect_start_input::at_entity(e.subject),
			predictable_only_by(e.subject)
		);
	}
}