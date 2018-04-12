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
#include "game/messages/queue_destruction.h"
#include "game/messages/damage_message.h"
#include "game/messages/melee_swing_response.h"
#include "game/messages/health_event.h"
#include "game/messages/exhausted_cast_message.h"

void sound_existence_system::create_sounds_from_game_events(const logic_step step) const {
	const auto& collisions = step.get_queue<messages::collision_message>();
	const auto& gunshots = step.get_queue<messages::gunshot_response>();
	const auto& damages = step.get_queue<messages::damage_message>();
	const auto& healths = step.get_queue<messages::health_event>();
	const auto& exhausted_casts = step.get_queue<messages::exhausted_cast>();
	const auto& metas = step.get_logical_assets().physical_materials;

	auto& cosmos = step.get_cosmos();

	for (size_t i = 0; i < collisions.size(); ++i) {
		const auto& c = collisions[i];

		if (c.type == messages::collision_message::event_type::POST_SOLVE) {
			const auto subject = cosmos[c.subject];
			const auto collider = cosmos[c.collider];

			const auto subject_coll = subject.get<invariants::fixtures>();
			const auto collider_coll = collider.get<invariants::fixtures>();

			const auto* const subject_coll_material = mapped_or_nullptr(metas, subject_coll.material);
			const auto* const collider_coll_material = mapped_or_nullptr(metas, collider_coll.material);

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
					sound_effect_input effect;
					effect.modifier.pitch = std::min(1.5f, 0.85f + pitch_mult);
					
					effect.modifier.gain = gain_mult;
					effect.id = sound_id;

					// LOG("Coll. gain/pitch: %f3/%f3", in.effect.modifier.gain, in.effect.modifier.pitch);

					effect.start(
						step, 
						sound_effect_start_input::fire_and_forget(c.point)
					);
				}
			}

			// skip the next, swapped collision message
			++i;
		}
	}

	for (const auto& g : gunshots) {
		{
			const auto subject = cosmos[g.subject];
			const auto& gun_def = subject.get<invariants::gun>();
			const auto gun_transform = subject.get_logic_transform();
			const auto owning_capability = subject.get_owning_transfer_capability();

			{
				const auto& effect = gun_def.muzzle_shot_sound;

				effect.start(
					step,
					sound_effect_start_input::at_entity(subject).set_listener(owning_capability)
				);
			}

			{

				const auto cued_count = gun_def.num_last_bullets_to_trigger_low_ammo_cue;

				if (cued_count > 0) {
					const auto ammo_info = get_ammunition_information(subject);

					if (ammo_info.total_charges < cued_count) {
						auto effect = gun_def.low_ammo_cue_sound;

						if (ammo_info.total_charges == cued_count - 1) {
							effect.modifier.gain *= 0.65f;
						}

						effect.start(
							step,
							sound_effect_start_input::fire_and_forget(gun_transform).set_listener(owning_capability)
						);
					}
				}
			}
		}
	}

	for (const auto& h : healths) {
		const auto subject = cosmos[h.subject];
		const auto& sentience = subject.get<invariants::sentience>();

		sound_effect_input effect;

		if (h.target == messages::health_event::target_type::HEALTH) {
			if (h.special_result == messages::health_event::result_type::DEATH) {
				effect = sentience.death_sound;
			}
			else if (h.effective_amount > 0) {
				effect = sentience.health_decrease_sound;
			}
			else {
				continue;
			}
		}
		else if (h.target == messages::health_event::target_type::PERSONAL_ELECTRICITY) {
			if (h.effective_amount > 0.f) {
				effect = cosmos.get_common_assets().ped_shield_impact_sound;
				effect.modifier.pitch *= 1.2f + h.effective_amount / 100.f;

				if (h.special_result == messages::health_event::result_type::PERSONAL_ELECTRICITY_DESTRUCTION) {
					effect = cosmos.get_common_assets().ped_shield_destruction_sound;
					effect.modifier.pitch *= 1.5f;
				}
			}
			else {
				continue;
			}
		}
		else if (h.target == messages::health_event::target_type::CONSCIOUSNESS) {
			if (h.effective_amount > 0.f) {
				effect = sentience.consciousness_decrease_sound;
				effect.modifier.pitch *= 1.2f + h.effective_amount / 100.f;

				if (h.special_result == messages::health_event::result_type::LOSS_OF_CONSCIOUSNESS) {
					effect = sentience.loss_of_consciousness_sound;
					effect.modifier.pitch *= 1.5f;
				}
			}
			else {
				continue;
			}
		}
		else {
			continue;
		}

		effect.start(
			step,
			sound_effect_start_input::at_listener(subject)
		);
	}

	for (const auto& d : damages) {
		if (d.inflictor_destructed) {
			const auto inflictor = cosmos[d.inflictor];
			
			const auto& effect = inflictor.get<invariants::missile>().destruction_sound;

			effect.start(
				step,
				sound_effect_start_input::orbit_absolute(cosmos[d.subject], d.point_of_impact).set_listener(d.subject)
			);
		}
	}

	for (const auto& e : exhausted_casts) {
		const auto& effect = cosmos.get_common_assets().cast_unsuccessful_sound;

		effect.start(
			step,
			sound_effect_start_input::at_listener(e.subject)
		);
	}
}