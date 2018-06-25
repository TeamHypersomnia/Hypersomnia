#include "missile_system.h"
#include "augs/math/steering.h"
#include "game/transcendental/cosmos.h"
#include "game/transcendental/entity_id.h"

#include "game/messages/collision_message.h"
#include "game/messages/queue_destruction.h"
#include "game/messages/damage_message.h"

#include "game/detail/inventory/perform_transfer.h"
#include "game/detail/entity_scripts.h"

#include "game/components/missile_component.h"
#include "game/components/rigid_body_component.h"
#include "game/components/transform_component.h"
#include "game/components/driver_component.h"
#include "game/components/fixtures_component.h"
#include "game/detail/view_input/sound_effect_input.h"
#include "game/components/sentience_component.h"
#include "game/components/attitude_component.h"
#include "game/components/sender_component.h"
#include "game/components/explosive_component.h"

#include "game/transcendental/entity_handle.h"
#include "game/transcendental/logic_step.h"
#include "game/transcendental/data_living_one_step.h"

#include "game/detail/physics/physics_scripts.h"

#include "game/assets/ids/asset_ids.h"

#include "game/enums/filters.h"

#include "game/stateless_systems/sound_existence_system.h"
#include "game/detail/organisms/startle_nearbly_organisms.h"

using namespace augs;

static void detonate_if_explosive(
	const logic_step step,
	const vec2 location,
	const const_entity_handle missile
) {
	if (const auto explosive = missile.find<invariants::explosive>()) {
		explosive->explosion.instantiate(step, location, entity_id());
	}
}

void missile_system::detonate_colliding_missiles(const logic_step step) {
	auto& cosm = step.get_cosmos();
	const auto delta = step.get_delta();
	const auto now = cosm.get_timestamp();
	const auto& events = step.get_queue<messages::collision_message>();

	for (const auto& it : events) {
		if (it.type != messages::collision_message::event_type::BEGIN_CONTACT || it.one_is_sensor) {
			continue;
		}

		const auto subject_handle = cosm[it.subject];
		const auto missile_handle = cosm[it.collider];

		missile_handle.dispatch_on_having<invariants::missile>([&](const auto typed_missile) {
			const auto& missile_def = typed_missile.template get<invariants::missile>();
			const auto& sender = typed_missile.template get<components::sender>();
			auto& missile = typed_missile.template get<components::missile>();

			const bool bullet_colliding_with_any_subject_of_sender = sender.is_sender_subject(subject_handle);
			const auto collision_normal = vec2(it.normal).normalize();
			
			const bool should_send_damage =
				!bullet_colliding_with_any_subject_of_sender
				&& missile_def.damage_upon_collision
				&& missile.damage_charges_before_destruction > 0
			;

			const auto impact_velocity = typed_missile.get_effective_velocity();
			const auto hit_facing = impact_velocity.degrees_between(collision_normal);

			LOG_NVPS(hit_facing);

			if (hit_facing > 80 && hit_facing < 100) {
				LOG("RICO");
			}

			if (should_send_damage) {
				if (missile_def.impulse_upon_hit > 0.f) {
					auto considered_impulse = missile_def.impulse_upon_hit * missile.power_multiplier_of_sender;

					if (const auto sentience = subject_handle.find<components::sentience>()) {
						const auto& shield_of_victim = sentience->get<electric_shield_perk_instance>();

						if (!shield_of_victim.timing.is_enabled(now, delta)) {
							considered_impulse *= missile_def.impulse_multiplier_against_sentience;
						}
					}

					const auto subject_of_impact = subject_handle.get_owner_of_colliders().get<components::rigid_body>();
					const auto subject_of_impact_mass_pos = subject_of_impact.get_mass_position(); 

					const auto impact = vec2(impact_velocity).set_length(considered_impulse);

					subject_of_impact.apply_impulse(impact, it.point - subject_of_impact_mass_pos);
				}

				missile.saved_point_of_impact_before_death = it.point;

				const auto owning_capability = subject_handle.get_owning_transfer_capability();

				const bool is_victim_an_item = subject_handle.has<components::item>();
				const bool is_victim_a_held_item = is_victim_an_item && owning_capability.alive() && owning_capability != it.subject;

				messages::damage_message damage_msg;
				damage_msg.subject_b2Fixture_index = it.subject_b2Fixture_index;
				damage_msg.collider_b2Fixture_index = it.collider_b2Fixture_index;

				if (is_victim_a_held_item) {
					missile_def.pass_through_held_item_sound.start(
						step,
						sound_effect_start_input::fire_and_forget( { it.point, 0.f } ).set_listener(owning_capability)
					);
				}

				const auto total_damage_amount = 
					missile_def.damage_amount * 
					missile.power_multiplier_of_sender
				;

				if (!is_victim_an_item && missile_def.destroy_upon_damage) {
					missile.damage_charges_before_destruction--;
					
					detonate_if_explosive(step, it.point, missile_handle);

					if (augs::is_positive_epsilon(total_damage_amount)) {
						startle_nearby_organisms(cosm, it.point, total_damage_amount * 12.f, 27.f, startle_type::LIGHTER);
					}

					// delete only once
					if (0 == missile.damage_charges_before_destruction) {
						step.post_message(messages::queue_destruction(it.collider));
						damage_msg.inflictor_destructed = true;

						auto rng = cosm.get_rng_for(typed_missile);
						auto flavours = missile_def.remnant_flavours;
						
						shuffle_range(flavours, rng.generator);
						auto how_many_along_normal = 3;//rng.randval(2u, 3u);

						for (const auto& r_id : flavours) {
							if (r_id.is_set()) {
								const auto speed = rng.randval(1000.f, 4800.f);
								const auto impact_dir = vec2(impact_velocity).normalize();

								vec2 vel;

								if (how_many_along_normal) {
									const auto sgn = rng.randval(0, 1);

									vel = vec2(collision_normal).rotate(rng.randval(70.f, 80.f), vec2()) * speed;

									if (sgn == 1) {
										vel = -vel;
									}
									--how_many_along_normal;
								}
								else {
									vel = vec2(impact_dir).rotate(rng.randval(-40.f, 40.f), vec2()) * speed;
								}

								cosmic::create_entity(
									cosm,
									r_id,
									[&](const auto typed_remnant) {
										auto spawn_offset = vec2(vel).normalize() * rng.randval(35.f, 50.f);
										const auto rot = rng.randval(0, 360);
										
										typed_remnant.set_logic_transform(transformr(it.point + spawn_offset, rot));
									},
									[&](const auto typed_remnant) {

										typed_remnant.template get<components::rigid_body>().set_velocity(vel);
										typed_remnant.template get<components::rigid_body>().set_angular_velocity(rng.randval(1060.f, 4000.f));

										const auto& effect = typed_remnant.template get<invariants::remnant>().trace_particles;

										effect.start(
											step,
											particle_effect_start_input::orbit_local(typed_remnant, { vec2::zero, 180 } )
										);
									}
								);
							}
						}
					}
				}

				damage_msg.inflictor = it.collider;
				damage_msg.subject = it.subject;
				damage_msg.amount = total_damage_amount;
				damage_msg.victim_shake = missile_def.victim_shake;
				damage_msg.impact_velocity = impact_velocity;
				damage_msg.point_of_impact = it.point;
				step.post_message(damage_msg);
			}
		});
	}
}

void missile_system::detonate_expired_missiles(const logic_step step) {
	auto& cosm = step.get_cosmos();
	const auto now = cosm.get_timestamp();
	const auto& delta = step.get_delta();

	cosm.for_each_having<components::missile>(
		[&](const auto it) {
			auto& missile = it.template get<components::missile>();
			auto& missile_def = it.template get<invariants::missile>();
		
			const bool already_detonated_in_this_step = 
				missile.damage_charges_before_destruction == 0
			;

			if (missile_def.constrain_lifetime && !already_detonated_in_this_step) {
				if (!missile.when_fired.was_set()) {
					missile.when_fired = now;
				}
				else {
					const auto fuse_delay_steps = static_cast<unsigned>(missile_def.max_lifetime_ms / delta.in_milliseconds());
					const auto when_detonates = missile.when_fired.step + fuse_delay_steps;

					if (/* should_already_detonate */ now.step >= when_detonates) {
						const auto current_pos = it.get_logic_transform().pos;

						missile.saved_point_of_impact_before_death = current_pos;
						detonate_if_explosive(step, current_pos, it);
						step.post_message(messages::queue_destruction(it));
					}
				}
			}

			const auto* const maybe_sender = it.template find<components::sender>();

			if (maybe_sender != nullptr && missile_def.homing_towards_hostile_strength > 0.f) {
				const auto sender_capability = cosm[maybe_sender->capability_of_sender];
				const auto sender_attitude = 
					sender_capability && sender_capability.template has<components::attitude>() ? sender_capability : entity_handle::dead_handle(cosm)
				;

				const auto particular_homing_target = cosm[missile.particular_homing_target];
				
				const auto detection_radius = 250.f;
				const auto closest_hostile = 
					particular_homing_target.alive() ? particular_homing_target : cosm[get_closest_hostile(it, sender_attitude, detection_radius, filters::bullet())]
				;

				const auto current_vel = it.template get<components::rigid_body>().get_velocity();
				const auto current_pos = it.get_logic_transform().pos;

				it.set_logic_transform({ current_pos, current_vel.degrees() });

				if (closest_hostile.alive()) {
					const auto hostile_pos = closest_hostile.get_logic_transform().pos;

					const auto homing_force = augs::seek( 
						current_vel,
						current_pos,
						hostile_pos,
						missile.initial_speed
					);

					it.template get<components::rigid_body>().apply_force(
						homing_force * missile_def.homing_towards_hostile_strength
					);
				}
			}
		}
	);
}