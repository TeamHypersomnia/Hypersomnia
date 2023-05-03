#include "sound_existence_system.h"
#include "game/cosmos/logic_step.h"
#include "game/cosmos/cosmos.h"
#include "game/cosmos/entity_handle.h"
#include "game/detail/entity_handle_mixins/get_owning_transfer_capability.hpp"
#include "game/cosmos/data_living_one_step.h"
#include "game/messages/queue_deletion.h"

#include "game/detail/entity_scripts.h"
#include "game/assets/all_logical_assets.h"

#include "game/components/missile_component.h"
#include "game/components/render_component.h"
#include "game/components/gun_component.h"
#include "game/components/fixtures_component.h"
#include "game/components/sentience_component.h"

#include "game/messages/start_sound_effect.h"
#include "game/messages/collision_message.h"
#include "game/messages/gunshot_message.h"
#include "game/messages/queue_deletion.h"
#include "game/messages/damage_message.h"
#include "game/messages/health_event.h"
#include "game/messages/exhausted_cast_message.h"
#include "game/detail/physics/calc_physical_material.hpp"
#include "game/detail/sentience/sentience_getters.h"

#include "game/detail/sentience/sentience_logic.h"
#include "augs/misc/randomization.h"
#include "game/detail/calc_ammo_info.hpp"

void play_collision_sound(
	const real32 strength,
	const vec2 location,
	const const_entity_handle sub,
	const const_entity_handle col,
	const logic_step step
) {
	const auto& cosm = step.get_cosmos();
	const auto& logicals = cosm.get_logical_assets();
	const auto subject_coll = sub.find<invariants::fixtures>();
	const auto collider_coll = col.find<invariants::fixtures>();

	if (subject_coll == nullptr || collider_coll == nullptr) {
		return;
	}

	const bool sub_missile = sub.find<invariants::missile>();
	const bool col_missile = col.find<invariants::missile>();

	const bool for_damage_cooldown = sub_missile || col_missile;

	const auto* const subject_coll_material = logicals.find(calc_physical_material(sub));
	const auto* const collider_coll_material = logicals.find(calc_physical_material(col));

	if (subject_coll_material != nullptr
		&& collider_coll_material != nullptr
	) {
		auto play_sound = [&](const auto* sound_def) {
			if (sound_def) {
				const auto impulse = strength * subject_coll->collision_sound_strength_mult * collider_coll->collision_sound_strength_mult;

				const auto gain_mult = impulse * impulse * sound_def->gain_mult;
				const auto pitch_mult = impulse * sound_def->pitch_mult;

				if (gain_mult > 0.01f) {
					// LOG("Cnorm/scgain/ccgain:\n%f4,%f4,%f4", strength, subject_coll.collision_sound_strength_mult, collider_coll.collision_sound_strength_mult);

					auto effect = sound_def->effect;

					const auto& pitch_bound = sound_def->pitch;
					effect.modifier.pitch *= std::max(pitch_bound.first, pitch_bound.second - pitch_mult);
					effect.modifier.gain *= std::min(1.f, gain_mult);

					auto start = sound_effect_start_input::fire_and_forget(location);

					if (for_damage_cooldown) {
						const auto missile = sub_missile ? sub : col;
						const auto wall = sub_missile ? col : sub;

						const auto capability = missile.template get<components::sender>().capability_of_sender;

						start.mark_as_missile_impact(capability, wall);
					}
					else {
						start.mark_source_collision(sub, col);
						start.collision_sound_cooldown_duration = sound_def->cooldown_duration;
						start.collision_sound_occurences_before_cooldown = sound_def->occurences_before_cooldown;
					}

					// TODO: PARAMETRIZE!
					effect.modifier.max_distance = 2700.f;
					effect.modifier.reference_distance = 700.f;

					/* TODO: properly determine predictability based on who thrown the item! */
					/* TODO: properly determine predictability based on if the collider's owning capability is a player! */

					effect.start(step, start, always_predictable_v);
				}
			}
		};

		const auto first_def = mapped_or_nullptr(subject_coll_material->collision_sound_matrix, collider_coll->material);
		const auto second_def = mapped_or_nullptr(collider_coll_material->collision_sound_matrix, subject_coll->material);

		play_sound(first_def);

		if (second_def != first_def) {
			play_sound(second_def);
		}
	}
}

void sound_existence_system::play_sounds_from_events(const logic_step step) const {
	const auto& collisions = step.get_queue<messages::collision_message>();
	const auto& gunshots = step.get_queue<messages::gunshot_message>();
	const auto& damages = step.get_queue<messages::damage_message>();
	const auto& healths = step.get_queue<messages::health_event>();
	const auto& exhausted_casts = step.get_queue<messages::exhausted_cast>();
	const auto& logicals = step.get_logical_assets();

	auto& cosm = step.get_cosmos();

	for (size_t i = 0; i < collisions.size(); ++i) {
		const auto& c = collisions[i];

		if (c.type == messages::collision_message::event_type::POST_SOLVE) {
			const auto subject = cosm[c.subject];
			const auto collider = cosm[c.collider];

			const auto collision_sound_strength = c.normal_impulse;

			if (subject.alive() && collider.alive()) {
				::play_collision_sound(collision_sound_strength, c.point, subject, collider, step);
			}

			// skip the next, swapped collision message
			++i;
		}
	}

	for (const auto& g : gunshots) {
		const auto subject = cosm[g.subject];

		subject.dispatch_on_having_all<invariants::gun>(
			[&](const auto& typed_subject) {
				const auto& gun_def = typed_subject.template get<invariants::gun>();

				const auto gun_transform = typed_subject.get_logic_transform();
				const auto owning_capability = cosm[g.capability];
				const auto predictability = predictable_only_by(owning_capability);

				{
					const auto& effect = gun_def.muzzle_shot_sound;

					effect.start(
						step,
						sound_effect_start_input::at_entity(typed_subject).set_listener(owning_capability),
						predictability
					);

					::mark_caused_danger(owning_capability, effect.modifier);
				}

				for (auto& r : g.spawned_rounds) {
					const auto spawned_round = cosm[r];

					{
						if (const auto missile = spawned_round.find<invariants::missile>()) {
							const auto& effect = missile->trace_sound;

							auto start = sound_effect_start_input::at_entity(cosm[r]);
							start.clear_when_target_entity_deleted = true;

							if (!missile->trace_sound_audible_to_shooter) {
								start.silent_trace_like = true;
								start.direct_listener = owning_capability;
							}

							effect.start(step, start, predictability);
						}
					}
				}

				{
					const auto cued_count = gun_def.num_last_bullets_to_trigger_low_ammo_cue;

					if (cued_count > 0) {
						const auto ammo_info = calc_ammo_info(typed_subject);

						if (ammo_info.total_charges <= cued_count) {
							auto effect = gun_def.low_ammo_cue_sound;

							if (ammo_info.total_charges == cued_count - 1) {
								effect.modifier.gain *= 0.65f;
							}

							effect.start(
								step,
								sound_effect_start_input::fire_and_forget(gun_transform).set_listener(owning_capability),
								predictability
							);
						}
					}
				}
			}
		);
	}

	for (const auto& h : healths) {
		const auto subject = cosm[h.subject];
		const auto& sentience = subject.get<invariants::sentience>();

		bool play_headshot = false;
		const bool was_headshot = h.origin.circumstances.headshot;

		sound_effect_input effect;

		auto predictability = always_predictable_v;

		if (h.target == messages::health_event::target_type::HEALTH) {
			if (h.special_result == messages::health_event::result_type::DEATH) {
				effect = sentience.death_sound;
				predictability = never_predictable_v;

				if (was_headshot) {
					play_headshot = true;
				}
			}
			else if (h.damage.total() > 0) {
				if (was_headshot) {
					play_headshot = true;
				}

				effect = h.was_dead ? sentience.corpse_health_decrease_sound : sentience.health_decrease_sound;
			}
			else {
				continue;
			}
		}
		else if (h.target == messages::health_event::target_type::PERSONAL_ELECTRICITY) {
			if (h.damage.total() > 0.f) {
				effect = cosm.get_common_assets().ped_shield_impact_sound;
				effect.modifier.pitch *= 1.2f + h.damage.total() / 100.f;

				if (h.special_result == messages::health_event::result_type::PERSONAL_ELECTRICITY_DESTRUCTION) {
					effect = cosm.get_common_assets().ped_shield_destruction_sound;
					effect.modifier.pitch *= 1.5f;

					predictability = never_predictable_v;
				}

				if (was_headshot) {
					play_headshot = true;
				}
			}
			else {
				continue;
			}
		}
		else if (h.target == messages::health_event::target_type::CONSCIOUSNESS) {
			if (h.damage.total() > 0.f) {
				effect = sentience.consciousness_decrease_sound;
				effect.modifier.pitch *= 1.2f + h.damage.total() / 100.f;

				if (h.special_result == messages::health_event::result_type::LOSS_OF_CONSCIOUSNESS) {
					effect = sentience.loss_of_consciousness_sound;
					effect.modifier.pitch *= 1.5f;

					predictability = never_predictable_v;
				}
			}
			else {
				continue;
			}
		}
		else {
			continue;
		}

		if (step.get_settings().effect_prediction.predict_death_sounds) {
			predictability = always_predictable_v;
		}

		auto start = sound_effect_start_input::at_listener(subject);

		if (h.special_result == messages::health_event::result_type::LOSS_OF_CONSCIOUSNESS) {
			start.clear_when_target_conscious = true;
		}

		if (h.special_result == messages::health_event::result_type::DEATH) {
			start.clear_when_target_alive = true;
		}

		if (effect.id.is_set()) {
			effect.start(
				step,
				start,
				predictability
			);
		}

		if (play_headshot) {
			auto hs_effect = sentience.headshot_sound;

			hs_effect.start(
				step,
				sound_effect_start_input::at_listener(subject),
				never_predictable_v
			);
		}
	}

	for (const auto& d : damages) {
		const auto subject = cosm[d.subject];

		if (subject.dead()) {
			continue;
		}

		const auto sender = d.origin.sender.capability_of_sender;

		auto mark_coll = [&](auto& eff) -> auto& {
			eff.mark_as_missile_impact(sender, d.subject);
			return eff;
		};

		if (subject.has<components::item>()) {
			if (const auto capability = subject.get_owning_transfer_capability()) {
				d.damage.pass_through_held_item_sound.start(
					step,
					mark_coll(sound_effect_start_input::fire_and_forget( { d.point_of_impact, 0.f } ).set_listener(capability)),
					always_predictable_v
				);

				return;
			}
		}

		{
			auto do_effect = [&](const auto& effect_def) {
				effect_def.sound.start(
					step,
					mark_coll(sound_effect_start_input::fire_and_forget(d.point_of_impact).set_listener(subject)),
					always_predictable_v
				);
			};

			const bool sentient = subject.has<components::sentience>();

			const auto& e = d.damage.effects;

			if (d.inflictor_destructed) {
				if (sentient) {
					if (e.sentience_impact.sound.id.is_set()) {
						// Bullet against body sound, but currently there are no sounds like this in the game
						do_effect(e.sentience_impact);
					}
				}
				else {
					// Bullet against wall sound
					auto eff = e.destruction;
					eff.sound.modifier.pitch *= step.step_rng.randval(0.8f, 1.2f);

					do_effect(eff);
				}
			}
			else {
				if (sentient) {
					if (e.sentience_impact.sound.id.is_set()) {
						// E.g. knife against body sound
						do_effect(e.sentience_impact);
					}
				}
				else {
					// E.g. knife against wall sound
					do_effect(e.impact);
				}
			}
		}

		if (d.type == adverse_element_type::FORCE) {
			const auto& fixtures = subject.get<invariants::fixtures>();

			if (const auto* const mat = logicals.find(fixtures.material)) {
				const auto unit = mat->unit_effect_damage;
				const auto mult = d.damage.base / unit;
				
				auto effect = mat->standard_damage_sound;
				effect.modifier.gain = std::min(1.f, mult);

				effect.start(
					step,
					mark_coll(sound_effect_start_input::fire_and_forget(d.point_of_impact).set_listener(subject)),
					always_predictable_v
				);
			}
		}
	}

	for (const auto& e : exhausted_casts) {
		const auto& effect = cosm.get_common_assets().cast_unsuccessful_sound;

		effect.start(
			step,
			sound_effect_start_input::at_listener(e.subject),
			predictable_only_by(e.subject)
		);
	}
}