#include "missile_system.h"
#include "augs/math/steering.h"
#include "game/cosmos/cosmos.h"
#include "game/cosmos/entity_id.h"
#include "game/cosmos/for_each_entity.h"

#include "game/messages/collision_message.h"
#include "game/messages/queue_deletion.h"
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

#include "game/cosmos/entity_handle.h"
#include "game/cosmos/logic_step.h"
#include "game/cosmos/data_living_one_step.h"
#include "game/cosmos/create_entity.hpp"
#include "game/detail/entity_handle_mixins/get_owning_transfer_capability.hpp"

#include "game/detail/physics/physics_scripts.h"

#include "game/assets/ids/asset_ids.h"

#include "game/enums/filters.h"

#include "game/stateless_systems/sound_existence_system.h"
#include "game/detail/organisms/startle_nearbly_organisms.h"
#include "game/detail/physics/missile_surface_info.h"
#include "game/detail/explosive/detonate.h"

#define USER_RICOCHET_COOLDOWNS 0
#define LOG_RICOCHETS 0

template <class... Args>
void RIC_LOG(Args&&... args) {
#if LOG_RICOCHETS
	LOG(std::forward<Args>(args)...);
#else
	((void)args, ...);
#endif
}

#if LOG_RICOCHETS
#define RIC_LOG_NVPS LOG_NVPS
#else
#define RIC_LOG_NVPS RIC_LOG
#endif

using namespace augs;

void play_collision_sound(
	const real32 strength,
	const vec2 location,
	const const_entity_handle sub,
	const const_entity_handle col,
	const logic_step step
);

template <class R, class F>
static void spawn_bullet_remnants(
	const logic_step step,
	R& rng,
	F flavours,
	const vec2& collision_normal,
	const vec2& impact_dir,
	const vec2& impact_point
) {
	auto& cosm = step.get_cosmos();

	shuffle_range(flavours, rng.generator);

	int total_spawned = std::min(static_cast<int>(flavours.size()), 2);

	auto how_many_along_normal = rng.randval(total_spawned - 1, total_spawned);

	for (int i = 0; i < total_spawned; ++i) {
		const auto& r_id = flavours[i];
		const auto speed = rng.randval(1000.f, 4800.f);

		vec2 vel;

		if (how_many_along_normal) {
			const auto sgn = rng.randval(0, 1);

			auto amount_rotated = rng.randval(70.f, 87.f);

			if (sgn == 1) {
				amount_rotated = -amount_rotated;
			}

			vel = vec2(collision_normal).rotate(amount_rotated) * speed;

			--how_many_along_normal;
		}
		else {
			vel = -1 * vec2(impact_dir).rotate(rng.randval(-40.f, 40.f)) * speed;
		}

		cosmic::create_entity(
			cosm,
			r_id,
			[&](const auto typed_remnant, auto&&...) {
				auto spawn_offset = vec2(vel).normalize() * rng.randval(55.f, 60.f);
				const auto rot = rng.randval(0, 360);
				
				typed_remnant.set_logic_transform(transformr(impact_point + spawn_offset, rot));
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


template <class T>
static void make_velocity_face_body_orientation(
	const T& typed_missile
) {
	const auto body = typed_missile.template get<components::rigid_body>();
	const auto current_vel = body.get_velocity();
	const auto tr = body.get_transform();

	const auto current_dir = vec2::from_degrees(tr.rotation);

	//const auto new_tr = transformr(tr.pos, current_vel.degrees());
	const auto new_vel = current_dir * current_vel.length();

	RIC_LOG_NVPS(new_vel);
	body.set_velocity(new_vel);
}

void missile_system::ricochet_missiles(const logic_step step) {
	auto& cosm = step.get_cosmos();
	const auto clk = cosm.get_clock();
	const auto now = clk.now;
	const auto& events = step.get_queue<messages::collision_message>();

	const auto steps = cosm.get_total_steps_passed();
	(void)steps;
	(void)now;

	for (const auto& it : events) {
		{
			const bool interested = it.type == messages::collision_message::event_type::BEGIN_CONTACT;

			if (!interested || it.one_is_sensor) {
				continue;
			}
		}

		const auto surface_handle = cosm[it.subject];
		const auto missile_handle = cosm[it.collider];

		missile_handle.dispatch_on_having_all<invariants::missile>([&](const auto& typed_missile) {
			RIC_LOG("(RIC)(%x) MISSILE %x WITH: %x", now.step, "CONTACT START", surface_handle);

			const auto& missile_def = typed_missile.template get<invariants::missile>();
			const auto ricochet_cooldown_ms = missile_def.ricochet_cooldown_ms;
			(void)missile_def;
			(void)ricochet_cooldown_ms;

			auto& missile = typed_missile.template get<components::missile>();

			const auto info = missile_surface_info(typed_missile, surface_handle);

			if (!info.is_ricochetable()) {
				RIC_LOG("non-ricochetable surface, IGNORED");
				return;
			}

			const auto collision_normal = vec2(it.normal).normalize();

			const auto impact_velocity = it.collider_impact_velocity;
			const auto impact_speed = impact_velocity.length();
			const auto impact_dir = impact_velocity / impact_speed;
			const auto impact_dot_normal = impact_dir.dot(collision_normal);

			if (impact_dot_normal >= 0.f) {
				RIC_LOG("dot normal is %x, IGNORED", impact_dot_normal);
				return;
			}

			/* 
				If the collision normal and velocity point in opposite directions,
				check if ricochet happens.
			*/

			const auto hit_facing = impact_dir.degrees_between(collision_normal);

			const auto& surface_fixtures = surface_handle.get<invariants::fixtures>();
			const auto max_ricochet_angle = surface_fixtures.max_ricochet_angle;

			const auto left_b = 90 - max_ricochet_angle;
			const auto right_b = 90 + max_ricochet_angle;

			if (DEBUG_DRAWING.draw_forces) {
				DEBUG_PERSISTENT_LINES.emplace_back(
					yellow,
					vec2(it.point),
					vec2(it.point) + collision_normal * 150
				);

				DEBUG_PERSISTENT_LINES.emplace_back(
					green,
					vec2(it.point),
					vec2(it.point) - impact_dir * 150
				);
			}

			RIC_LOG_NVPS(left_b, hit_facing, right_b);

			if (hit_facing > left_b && hit_facing < right_b) {
				{
					const bool born_cooldown = clk.lasts(
						missile_def.ricochet_born_cooldown_ms,
						typed_missile.when_born()
					);

					if (born_cooldown) {
						RIC_LOG("Rico: born cooldown");
						return;
					}

#if USER_RICOCHET_COOLDOWNS
					const bool ricochet_cooldown = clk.lasts(
						ricochet_cooldown_ms,
						missile.when_last_ricocheted
					);
#else
					const bool ricochet_cooldown = now.step <= missile.when_last_ricocheted.step + 1;
#endif
					if (ricochet_cooldown) {
						RIC_LOG("Rico: rico cooldown");
						return;
					}

					RIC_LOG("NO COOLDOWN, performing RICOCHET with %x, NOW: %x, impact dir: %x, coll normal: %x", surface_handle, now.step, impact_dir, collision_normal);
				}

				const auto angle = std::min(hit_facing - left_b, right_b - hit_facing);
				const auto angle_mult = angle / max_ricochet_angle;

				missile.when_last_ricocheted = now;

				const auto reflected_dir = vec2(impact_dir).reflect(collision_normal);
				const auto& rigid_body = typed_missile.template get<components::rigid_body>();

				const auto target_position = rigid_body.get_transform().pos;
				const auto new_transform = transformr(target_position, reflected_dir.degrees());

				const auto new_velocity = reflected_dir * impact_speed;

				RIC_LOG_NVPS(new_velocity);

				rigid_body.set_velocity(new_velocity);
				rigid_body.set_transform(new_transform);

				::play_collision_sound(angle_mult * 150.f, it.point, typed_missile, surface_handle, step);

				const auto effect_transform = transformr(it.point, reflected_dir.degrees());

				{
					const auto& effect = missile_def.ricochet_particles;

					effect.start(
						step,
						particle_effect_start_input::fire_and_forget(effect_transform)
					);
				}

				{
					const auto pitch = 0.7f + angle_mult / 1.5f;

					auto effect = missile_def.ricochet_sound;
					effect.modifier.pitch = pitch;

					effect.start(
						step,
						sound_effect_start_input::fire_and_forget(effect_transform)
					);
				}
			}
			else {
				RIC_LOG("Not enough facing. IGNORED.");
			}
		});
	}
}

void missile_system::detonate_colliding_missiles(const logic_step step) {
	auto& cosm = step.get_cosmos();
	const auto& clk = cosm.get_clock();
	const auto& now = clk.now;
	const auto& events = step.get_queue<messages::collision_message>();

	const auto steps = cosm.get_total_steps_passed();
	(void)steps;

	for (const auto& it : events) {
		const bool contact_start = it.type == messages::collision_message::event_type::BEGIN_CONTACT;
		const bool pre_solve = it.type == messages::collision_message::event_type::PRE_SOLVE;

		{
			const bool interested = contact_start || pre_solve;

			if (!interested || it.one_is_sensor) {
				continue;
			}
		}

		const auto surface_handle = cosm[it.subject];
		const auto missile_handle = cosm[it.collider];

		missile_handle.dispatch_on_having_all<invariants::missile>([&](const auto& typed_missile) {
			RIC_LOG("(DET) MISSILE %x WITH: %x", pre_solve ? "PRE SOLVE" : "CONTACT START", surface_handle);

			const auto& missile_def = typed_missile.template get<invariants::missile>();
			const auto ricochet_cooldown_ms = missile_def.ricochet_cooldown_ms;
			(void)missile_def;
			(void)ricochet_cooldown_ms;

			auto& missile = typed_missile.template get<components::missile>();

			const auto collision_normal = vec2(it.normal).normalize();
			const auto info = missile_surface_info(typed_missile, surface_handle);

			if (info.should_ignore_altogether()) {
				RIC_LOG("IGNORED");
				return;
			}

			const bool should_send_damage =
				missile_def.damage_upon_collision
				&& missile.damage_charges_before_destruction > 0
			;

			if (!should_send_damage) {
				return;
			}

			if (pre_solve) {
				/* With a PreSolve must have happened a PostSolve that altered our initial velocity. */

				make_velocity_face_body_orientation(typed_missile);
			}

			{

#if USER_RICOCHET_COOLDOWNS
				const bool ricochet_cooldown = clk.lasts(
					ricochet_cooldown_ms,
					missile.when_last_ricocheted
				);
#else
				const bool ricochet_cooldown = now.step <= missile.when_last_ricocheted.step + 1;
#endif
				if (ricochet_cooldown) {
					RIC_LOG("DET: rico cooldown, no impact");
					return;
				}

				RIC_LOG("DET: NO COOLDOWN, IMPACTING");
			}

			const auto impact_velocity = it.collider_impact_velocity;
			const auto impact_dir = vec2(impact_velocity).normalize();

			if (missile_def.impulse_upon_hit > 0.f && contact_start) {
				auto considered_impulse = missile_def.impulse_upon_hit * missile.power_multiplier_of_sender;

				if (const auto sentience = surface_handle.find<components::sentience>()) {
					const auto& shield_of_victim = sentience->get<electric_shield_perk_instance>();

					if (!shield_of_victim.timing.is_enabled(clk)) {
						considered_impulse *= missile_def.impulse_multiplier_against_sentience;
					}
				}

				const auto subject_of_impact = surface_handle.get_owner_of_colliders().get<components::rigid_body>();
				const auto subject_of_impact_mass_pos = subject_of_impact.get_mass_position(); 

				const auto impact = vec2(impact_velocity).set_length(considered_impulse);

				subject_of_impact.apply_impulse(impact, it.point - subject_of_impact_mass_pos);
			}

			missile.saved_point_of_impact_before_death = { it.point, impact_dir.degrees() };

			const auto owning_capability = surface_handle.get_owning_transfer_capability();
			const bool is_victim_a_held_item = info.surface_is_item && owning_capability.alive() && owning_capability != it.subject;

			messages::damage_message damage_msg;
			damage_msg.subject_b2Fixture_index = it.subject_b2Fixture_index;
			damage_msg.collider_b2Fixture_index = it.collider_b2Fixture_index;

			if (is_victim_a_held_item && contact_start) {
				missile_def.pass_through_held_item_sound.start(
					step,
					sound_effect_start_input::fire_and_forget( { it.point, 0.f } ).set_listener(owning_capability)
				);
			}

			const auto total_damage_amount = 
				missile_def.damage_amount * 
				missile.power_multiplier_of_sender
			;

			auto& charges = missile.damage_charges_before_destruction;
			const bool send_damage = charges > 0;

			if (info.should_detonate() && missile_def.destroy_upon_damage) {
				--charges;
				
				detonate_if(typed_missile.get_id(), it.point, step);

				if (augs::is_positive_epsilon(total_damage_amount)) {
					startle_nearby_organisms(cosm, it.point, total_damage_amount * 12.f, 27.f, startle_type::LIGHTER);
				}

				// delete only once
				if (charges == 0) {
					step.post_message(messages::queue_deletion(typed_missile));
					damage_msg.inflictor_destructed = true;

					auto rng = cosm.get_rng_for(typed_missile);

					spawn_bullet_remnants(
						step,
						rng,
						missile_def.remnant_flavours,
						collision_normal,
						impact_dir,
						it.point
					);
				}
			}

			if (send_damage && contact_start) {
				damage_msg.origin = damage_origin(typed_missile);
				damage_msg.subject = it.subject;
				damage_msg.amount = total_damage_amount;
				damage_msg.victim_shake = missile_def.victim_shake;
				damage_msg.victim_shake *= missile.power_multiplier_of_sender;
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
						const auto current_tr = it.get_logic_transform();

						missile.saved_point_of_impact_before_death = current_tr;
						detonate_if(it, current_tr.pos, step);
						step.post_message(messages::queue_deletion(it));
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
					particular_homing_target.alive() ? particular_homing_target : cosm[get_closest_hostile(it, sender_attitude, detection_radius, filters[predefined_filter_type::FLYING_ITEM])]
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