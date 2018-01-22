#include "sound_existence_system.h"
#include "game/transcendental/logic_step.h"
#include "game/transcendental/cosmos.h"
#include "game/transcendental/data_living_one_step.h"
#include "game/messages/queue_destruction.h"

#include "game/detail/entity_scripts.h"
#include "game/assets/all_logical_assets.h"

#include "game/components/missile_component.h"
#include "game/components/render_component.h"
#include "game/components/gun_component.h"
#include "game/components/fixtures_component.h"
#include "game/components/sentience_component.h"

#include "game/messages/collision_message.h"
#include "game/messages/gunshot_response.h"
#include "game/messages/create_particle_effect.h"
#include "game/messages/queue_destruction.h"
#include "game/messages/damage_message.h"
#include "game/messages/melee_swing_response.h"
#include "game/messages/health_event.h"
#include "game/messages/exhausted_cast_message.h"

bool components::sound_existence::is_activated(const const_entity_handle h) {
	return
		//h.get<tree_of_npo_node_input>().is_activated() && 
		h.get<components::processing>().is_in(processing_subjects::WITH_SOUND_EXISTENCE);
}

void components::sound_existence::activate(const entity_handle h) {
	if (is_activated(h)) {
		return;
	}

	auto& existence = h.get<components::sound_existence>();
	existence.time_of_birth = h.get_cosmos().get_timestamp();
	h.get<components::processing>().enable_in(processing_subjects::WITH_SOUND_EXISTENCE);
}

void components::sound_existence::deactivate(const entity_handle h) {
	if (!is_activated(h)) {
		return;
	}

	h.get<components::processing>().disable_in(processing_subjects::WITH_SOUND_EXISTENCE);
}

size_t components::sound_existence::random_variation_number_from_transform(const components::transform t) const {
	return time_of_birth.step + std::hash<vec2>()(t.pos);
}

float components::sound_existence::calculate_max_audible_distance() const {
	return input.effect.modifier.max_distance + input.effect.modifier.reference_distance;
}

void sound_existence_system::destroy_dead_sounds(const logic_step step) const {
	auto& cosmos = step.get_cosmos();
	const auto timestamp = cosmos.get_timestamp();

	cosmos.for_each(
		processing_subjects::WITH_SOUND_EXISTENCE,
		[&](const entity_handle it) {
			auto& existence = it.get<components::sound_existence>();

			const auto repetitions = existence.input.effect.modifier.repetitions;

			if (repetitions > -1 && (timestamp - existence.time_of_birth).step > existence.max_lifetime_in_steps * repetitions) {
				if (existence.input.delete_entity_after_effect_lifetime) {
					step.post_message(messages::queue_destruction(it));
				}
				else {
					components::sound_existence::deactivate(it);
				}
			}
		},
		subjects_iteration_flag::POSSIBLE_ITERATOR_INVALIDATION
	);
}

void sound_existence_system::create_sounds_from_game_events(const logic_step step) const {
	const auto& collisions = step.get_queue<messages::collision_message>();
	const auto& gunshots = step.get_queue<messages::gunshot_response>();
	const auto& damages = step.get_queue<messages::damage_message>();
	const auto& swings = step.get_queue<messages::melee_swing_response>();
	const auto& healths = step.get_queue<messages::health_event>();
	const auto& exhausted_casts = step.get_queue<messages::exhausted_cast>();
	auto& cosmos = step.get_cosmos();

	for (size_t i = 0; i < collisions.size(); ++i) {
		const auto& c = collisions[i];

		if (c.type == messages::collision_message::event_type::POST_SOLVE) {
			const auto subject = cosmos[c.subject];
			const auto collider = cosmos[c.collider];

			const auto subject_coll = subject.get_def<definitions::fixtures>();
			const auto collider_coll = collider.get_def<definitions::fixtures>();

			const auto* const subject_coll_material = step.get_logical_assets().find(subject_coll.material);
			const auto* const collider_coll_material = step.get_logical_assets().find(collider_coll.material);

			if (
				subject_coll_material != nullptr
				&& collider_coll_material != nullptr
			) {
				const auto sound_id = subject_coll_material->collision_sound_matrix.at(collider_coll.material);

				const auto impulse = (c.normal_impulse) * subject_coll.collision_sound_gain_mult * collider_coll.collision_sound_gain_mult;

				// LOG("Cnorm/scgain/ccgain:\n%f4,%f4,%f4", c.normal_impulse, subject_coll.collision_sound_gain_mult, collider_coll.collision_sound_gain_mult);

				const auto gain_mult = (impulse / 15.f) * (impulse / 15.f);
				const auto pitch_mult = impulse / 185.f;

				if (gain_mult > 0.01f) {
					sound_existence_input in;
					in.delete_entity_after_effect_lifetime = true;
					in.effect.modifier.pitch = std::min(1.5f, 0.85f + pitch_mult);
					
					in.effect.modifier.gain = gain_mult;
					in.effect.id = sound_id;

					// LOG("Coll. gain/pitch: %f3/%f3", in.effect.modifier.gain, in.effect.modifier.pitch);

					in.create_sound_effect_entity(step, c.point, entity_id()).add_standard_components(step);
				}
			}

			// skip the next, swapped collision message
			++i;
		}
	}

	for (const auto& g : gunshots) {
		for (const auto r : g.spawned_rounds) {
			const auto subject = cosmos[r];

			auto& missile = subject.get<components::missile>();

			sound_existence_input in;
			in.effect = missile.trace_sound;

			missile.trace_sound_entity = in.create_sound_effect_entity(
				step,
				g.muzzle_transform,
				r
			).add_standard_components(step);
		}

		{
			const auto subject = cosmos[g.subject];
			const auto& gun = subject.get<components::gun>();
			const auto& gun_def = subject.get_def<definitions::gun>();
			const auto gun_transform = subject.get_logic_transform();
			const auto owning_capability = subject.get_owning_transfer_capability();

			{
				sound_existence_input in;
				in.effect = gun_def.muzzle_shot_sound;
				in.direct_listener = owning_capability;

				in.create_sound_effect_entity(step, subject.get_logic_transform(), entity_id()).add_standard_components(step);
			}

			{

				const auto cued_count = gun_def.num_last_bullets_to_trigger_low_ammo_cue;

				if (cued_count > 0) {
					const auto ammo_info = get_ammunition_information(subject);

					if (ammo_info.total_charges < cued_count) {
						sound_existence_input in;
						in.effect = gun_def.low_ammo_cue_sound;

						if (ammo_info.total_charges == cued_count - 1) {
							in.effect.modifier.gain *= 0.65f;
						}

						in.direct_listener = owning_capability;

						in.create_sound_effect_entity(
							step,
							gun_transform,
							entity_id()
						).add_standard_components(step);
					}
				}
			}
		}
	}

	for (const auto& h : healths) {
		const auto subject = cosmos[h.subject];
		const auto& sentience = subject.get<components::sentience>();

		sound_existence_input in;
		in.direct_listener = subject;

		if (h.target == messages::health_event::target_type::HEALTH) {
			if (h.special_result == messages::health_event::result_type::DEATH) {
				in.effect = sentience.death_sound;
			}
			else if (h.effective_amount > 0) {
				in.effect = sentience.health_decrease_sound;
			}
			else {
				continue;
			}
		}
		else if (h.target == messages::health_event::target_type::PERSONAL_ELECTRICITY) {
			if (h.effective_amount > 0.f) {
				in.effect = cosmos.get_common_assets().ped_shield_impact_sound;
				in.effect.modifier.pitch *= 1.2f + h.effective_amount / 100.f;

				if (h.special_result == messages::health_event::result_type::PERSONAL_ELECTRICITY_DESTRUCTION) {
					in.effect = cosmos.get_common_assets().ped_shield_destruction_sound;
					in.effect.modifier.pitch *= 1.5f;
				}
			}
			else {
				continue;
			}
		}
		else if (h.target == messages::health_event::target_type::CONSCIOUSNESS) {
			if (h.effective_amount > 0.f) {
				in.effect = sentience.consciousness_decrease_sound;
				in.effect.modifier.pitch *= 1.2f + h.effective_amount / 100.f;

				if (h.special_result == messages::health_event::result_type::LOSS_OF_CONSCIOUSNESS) {
					in.effect = sentience.loss_of_consciousness_sound;
					in.effect.modifier.pitch *= 1.5f;
				}
			}
			else {
				continue;
			}
		}
		else {
			continue;
		}

		in.create_sound_effect_entity(
			step, 
			subject.get_logic_transform(), 
			subject
		).add_standard_components(step);
	}

	for (const auto& d : damages) {
		if (d.inflictor_destructed) {
			const auto inflictor = cosmos[d.inflictor];
			
			sound_existence_input in;
			in.direct_listener = d.subject;
			in.effect = inflictor.get<components::missile>().destruction_sound;
			
			in.create_sound_effect_entity(
				step, 
				d.point_of_impact, 
				entity_id()
			).add_standard_components(step);
		}
	}

	for (const auto& e : exhausted_casts) {
		const auto subject = cosmos[e.subject];

		sound_existence_input in;
		in.direct_listener = e.subject;
		in.effect = cosmos.get_common_assets().cast_unsuccessful_sound;

		in.create_sound_effect_entity(
			step, 
			e.transform, 
			entity_id()
		).add_standard_components(step);
	}
}