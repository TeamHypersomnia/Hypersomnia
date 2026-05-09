#include "game/detail/sentience/sentience_logic.h"
#include "game/cosmos/cosmos.h"
#include "game/cosmos/entity_handle.h"
#include "game/stateless_systems/driver_system.h"
#include "game/detail/inventory/drop_from_all_slots.h"
#include "game/cosmos/create_entity.hpp"
#include "game/cosmos/data_living_one_step.h"
#include "game/detail/explosive/detonate.h"
#include "game/detail/sentience/gore/blood_splatter.hpp"
#include "game/detail/sentience/gore/idle_splatter.hpp"
#include "game/messages/pure_color_highlight_message.h"
#include "game/messages/start_particle_effect.h"
#include "game/detail/view_input/sound_effect_input.h"
#include "game/components/owner_component.h"

#include "augs/log.h"

static constexpr real32 arm_detach_damage_offset = 90.f;

static void try_detach_arms(
	const allocate_new_entity_access access,
	const logic_step step,
	const entity_handle subject,
	components::sentience& sentience,
	const invariants::sentience& sentience_def,
	const vec2 point_of_impact,
	const real32 damage_amount,
	const damage_origin& origin
) {
	if (!sentience.is_dead()) {
		return;
	}

	/* Cap: max 2 arms total */
	if (sentience.arms_queued_for_detach >= 2) {
		return;
	}

	const auto& health = sentience.get<health_meter_instance>();
	const auto current_health = health.value;

	const bool was_headshot_kill = sentience.knockout_origin.circumstances.headshot;
	const auto base = was_headshot_kill ? sentience.health_value_at_death : 0.f;

	const auto effective_offset = sentience.has_exploded ? arm_detach_damage_offset * 1.5f : arm_detach_damage_offset;
	const auto threshold_1 = base - effective_offset;
	const auto threshold_2 = base - effective_offset * 2.f;

	const int arms_should_be_detached = 
		current_health <= threshold_2 ? 2 :
		current_health <= threshold_1 ? 1 : 0;

	if (arms_should_be_detached <= sentience.arms_queued_for_detach) {
		return;
	}

	if (!sentience_def.detached_flavours.arm_upper.is_set()) {
		return;
	}

	const auto subject_transform = subject.get_logic_transform();

	/*
		Use character facing to determine which arm is upper vs lower and which
		direction each arm flies. perp_cw of facing-right = screen-down, so negate
		to get screen-up. This stays constant regardless of which side the bullet
		comes from, so upper arm always flies up and lower arm always flies down
		relative to the character's orientation.
	*/
	const auto facing_up = -vec2::from_degrees(subject_transform.rotation).perpendicular_cw();

	auto determine_arm_is_upper = [&]() -> bool {
		if (point_of_impact.is_nonzero()) {
			const auto body_center = subject_transform.pos;
			const auto to_impact = point_of_impact - body_center;
			const auto dot = to_impact.dot(facing_up);
			return dot < 0;
		}
		return true;
	};

	/* Only detach one arm per call (fix for one-shot peeling off two arms) */
	const int currently_queued = sentience.arms_queued_for_detach;
	const bool is_first_arm = (currently_queued == 0);

	/*
		First arm direction is determined by impact position.
		Second arm always goes in the opposite direction of the first.
	*/
	const bool is_upper = is_first_arm ? determine_arm_is_upper() : !sentience.first_arm_queued_as_upper;
	const auto fly_direction = is_upper ? facing_up : -facing_up;
	const auto arm_flavour = is_upper ? sentience_def.detached_flavours.arm_lower : sentience_def.detached_flavours.arm_upper;

	/* Damage-based speed scaling: 0 damage = 0%, 100 damage = 100% of base speed */
	const auto damage_ratio = repro::sqrt(std::min(1.0f, damage_amount / 100.0f));
	const auto arm_velocity = fly_direction * (sentience_def.base_detached_arm_speed * damage_ratio);
	auto arm_transform = subject_transform;
	arm_transform.pos = point_of_impact;

	const auto typed_subject_id = subject.get_id();
	const auto head_effect = sentience_def.detached_head_particles;

	/*
		Precompute the splatter origin for the immediate arm splatter.
		This will be spawned in the post_construction callback using the arm entity as orbit subject.
	*/
	const auto arm_splatter_origin = point_of_impact.is_nonzero() ? point_of_impact : subject_transform.pos;
	const auto arm_detach_sound = sentience_def.arm_detach_sound;

	/* Increment immediately to prevent duplicate spawns before post_construction fires */
	if (is_first_arm) {
		sentience.first_arm_queued_as_upper = is_upper;
	}
	sentience.arms_queued_for_detach++;
	
	/*
		Only award coins for arm detachment if the damage comes from a different faction.
		This prevents players from teamkilling to get the award from detaching arms (gore).
	*/
	const auto victim_faction = subject.get_official_faction();

	if (origin.sender.faction_of_sender != victim_faction || victim_faction == faction_type::SPECTATOR) {
		sentience.coins_on_body += 500;
	}

	cosmic::queue_create_entity(
		step,
		arm_flavour,
		[arm_transform, arm_velocity, typed_subject_id](const auto& typed_entity, auto&) {
			typed_entity.set_logic_transform(arm_transform);

			const auto& rigid_body = typed_entity.template get<components::rigid_body>();
			rigid_body.set_velocity(arm_velocity);
			rigid_body.set_angular_velocity(700.f);
			rigid_body.get_special().during_cooldown_ignore_collision_with = typed_subject_id;
		},

		[head_effect, typed_subject_id, is_upper, access, arm_splatter_origin, fly_direction, arm_detach_sound](const auto& typed_entity, const logic_step step) {
			if (const auto typed_subject = step.get_cosmos()[typed_subject_id]) {
				auto& s = typed_subject.template get<components::sentience>();

				if (is_upper) {
					s.detached.arm_upper = typed_entity;
				}
				else {
					s.detached.arm_lower = typed_entity;
				}

				s.when_arms_detached = step.get_cosmos().get_timestamp();
				s.pending_arm_splatters += 2;
			}

			const auto predictability = 
				step.get_settings().effect_prediction.predict_death_particles 
				? always_predictable_v
				: never_predictable_v
			;

			head_effect.start(
				step,
				particle_effect_start_input::orbit_local(typed_entity, { vec2::zero, 180 } ),
				predictability
			);

			/*
				Send a white highlight for the detached arm when it first appears.
				Analogous to the lying corpse highlight.
			*/
			{
				messages::pure_color_highlight msg;
				msg.subject = typed_entity;
				msg.input.starting_alpha_ratio = 1.f;
				msg.input.maximum_duration_seconds = 0.4f;
				msg.input.size_mult_start = 2.8f;
				msg.input.color = white;

				step.post_message(msg);
			}

			/*
				Play the arm detach sound from the dead player's perspective.
			*/
			if (arm_detach_sound.id.is_set()) {
				const auto predictability =
					step.get_settings().effect_prediction.predict_death_particles
					? always_predictable_v
					: never_predictable_v
				;

				arm_detach_sound.start(
					step,
					sound_effect_start_input::at_listener(typed_subject_id),
					predictability
				);
			}

			/*
				Spawn an immediate splatter at the damage location,
				oriented in the flight direction.
				Use typed_entity (arm) as orbit subject to avoid orbit glitches.

				Kept last because spawn_blood_splatter allocates an entity in the
				decal_decoration pool. If the arm flavour ever shared that pool,
				typed_entity's cached pointer would be invalidated by this allocation.
			*/

			{
				auto& cosm = step.get_cosmos();
				auto rng = cosm.get_rng_for(typed_entity.get_id());
				::spawn_blood_splatter(access, rng, step, typed_entity, arm_splatter_origin + fly_direction * 20.f, arm_splatter_origin, 0.7f);
			}
		}
	);
}

void handle_corpse_damage(
	const allocate_new_entity_access access,
	const logic_step step,
	const entity_handle subject,
	components::sentience& sentience,
	const invariants::sentience& sentience_def,
	const vec2 impact_direction,
	const vec2 point_of_impact,
	const real32 damage_amount,
	const damage_origin& origin
) {
	/*
		Only update the damage direction if it hasn't been set yet.
		This preserves the direction of the KILLING blow.
	*/
	if (impact_direction.is_nonzero() && sentience.last_corpse_damage_direction.is_zero()) {
		sentience.last_corpse_damage_direction = impact_direction;
	}

	auto& health = sentience.get<health_meter_instance>();
	auto& when_ignited = sentience.when_corpse_catched_fire;

	const auto& cosm = subject.get_cosmos();
	const auto now = cosm.get_timestamp();

	auto ignite_corpse = [&]() {
		when_ignited = now;

		{
			const auto& effect = sentience_def.corpse_catch_fire_particles;

			effect.start(
				step,
				particle_effect_start_input::orbit_local(subject, { vec2::zero, 180 } ),
				never_predictable_v
			);
		}

		{
			const auto& effect = sentience_def.corpse_catch_fire_sound;

			effect.start(
				step,
				sound_effect_start_input::at_listener(subject),
				never_predictable_v
			);
		}
	};

	const auto damage_past_breaking_point = -health.value - sentience_def.damage_required_for_corpse_explosion;
	const bool is_ignited = when_ignited.was_set();

	if (damage_past_breaking_point > 0 && !sentience.has_exploded) {
		if (!is_ignited) {
			ignite_corpse();
		}
	}

	/*
		Post a damage highlight on the lying corpse so
		the hit flash is visible on the physical body.
	*/
	if (sentience.has_exploded && damage_amount > 0) {
		if (const auto lying_corpse = cosm[sentience.detached.lying_corpse]) {
			messages::pure_color_highlight msg;
			msg.subject = lying_corpse.get_id();
			msg.input.starting_alpha_ratio = 1.f;
			msg.input.color = white;
			msg.input.size_mult_start = 1.5f;
			msg.input.maximum_duration_seconds = 0.12f;

			step.post_message(msg);
		}
	}

	const auto prev_arms_queued = sentience.arms_queued_for_detach;

	try_detach_arms(access, step, subject, sentience, sentience_def, point_of_impact, damage_amount, origin);

	/*
		If an arm was just detached while the lying corpse already exists,
		replace it with the correct sprite variant (noarm/noarms).
	*/
	const auto new_arms = sentience.arms_queued_for_detach;

	if (new_arms != prev_arms_queued && sentience.detached.lying_corpse.is_set()) {
		sentience.when_lying_corpse_replaced = cosm.get_timestamp();

		const auto lying_corpse = cosm[sentience.detached.lying_corpse];

		if (lying_corpse) {
			const auto new_flavour = [&]() {
				if (new_arms >= 2) {
					return sentience_def.lying_corpse_noarms_flavour;
				}
				if (new_arms >= 1) {
					return sentience_def.lying_corpse_noarm_flavour;
				}
				return sentience_def.lying_corpse_flavour;
			}();

			if (new_flavour.is_set()) {
				const auto old_transform = lying_corpse.get_logic_transform();
				const auto old_velocity = lying_corpse.get_effective_velocity();

				const auto old_flip = [&]() {
					auto result = flip_flags();
					lying_corpse.dispatch([&](const auto& typed) {
						if (const auto* geo = typed.get().template find<components::overridden_geo>()) {
							result = geo->flip;
						}
					});
					return result;
				}();

				/*
					Recompute flip based on current arm detachment state.
					Same logic as the initial lying corpse spawn.
				*/
				const bool should_flip = [&]() {
					if (new_arms == 1) {
						return sentience.should_flip_tattered_sprite();
					}
					return old_flip.vertically;
				}();

				const auto typed_subject_id = subject.get_id();
				const auto old_lying_corpse_id = lying_corpse.get_id();

				/*
					Precompute the head offset for the replacement corpse sprite
					so we can retarget the detached_head_particles stream.
				*/
				const bool head_detached =
					sentience.detached.head.is_set()
					|| sentience.knockout_origin.circumstances.headshot
				;

				const auto head_effect = sentience_def.detached_head_particles;

				const auto corpse_head_offset = [&]() {
					if (head_detached && head_effect.id.is_set()) {
						const auto corpse_image_id = cosm.on_flavour(
							new_flavour,
							[](const auto& f) { return f.get_image_id(); }
						);

						if (corpse_image_id.is_set()) {
							auto head_off = cosm.get_logical_assets().get_offsets(corpse_image_id).torso.head;

							if (should_flip) {
								head_off.flip_vertically();
							}

							return head_off;
						}
					}

					return transformi();
				}();

				step.queue_deletion_of(lying_corpse, "Replacing lying corpse with updated arm variant");

				/*
					Update pending gore splatters for the newly detached arm(s).
				*/
				if (new_arms >= 1 && prev_arms_queued < 1) {
					sentience.pending_lying_gore_shoulder = 3;
				}

				if (new_arms >= 2 && prev_arms_queued < 2) {
					sentience.pending_lying_gore_secondary_shoulder = 3;
				}

				/*
					Reset the settlement timer so gore dripping re-evaluates
					with the new corpse.
				*/
				sentience.when_lying_corpse_settled = {};

				cosmic::queue_create_entity(
					step,
					new_flavour,
					[old_transform, old_velocity, should_flip, typed_subject_id](const auto& typed_entity, auto& agg) {
						typed_entity.set_logic_transform(old_transform);

						const auto& rigid_body = typed_entity.template get<components::rigid_body>();
						rigid_body.set_velocity(old_velocity);
						rigid_body.set_angular_velocity(0.f);

						if (should_flip) {
							if (auto* geo = agg.template find<components::overridden_geo>()) {
								geo->flip.vertically = true;
							}
						}

						if (auto* owner_comp = agg.template find<components::owner>()) {
							owner_comp->owner_body = typed_subject_id;
						}
					},

					[typed_subject_id, old_lying_corpse_id, head_detached, head_effect, corpse_head_offset](const auto& typed_entity, const logic_step step) {
						if (const auto typed_subject = step.get_cosmos()[typed_subject_id]) {
							auto& s = typed_subject.template get<components::sentience>();
							s.detached.lying_corpse = typed_entity;
						}

						/*
							Send a white highlight for the replacement lying corpse.
						*/
						{
							messages::pure_color_highlight msg;
							msg.subject = typed_entity;
							msg.input.starting_alpha_ratio = 1.f;
							msg.input.color = white;
							msg.input.size_mult_start = 1.85f;
							msg.input.maximum_duration_seconds = 0.2f;

							step.post_message(msg);
						}

						/*
							Retarget the detached_head_particles stream
							from the old lying corpse to the replacement.
						*/
						if (head_detached && head_effect.id.is_set()) {
							const auto predictability =
								step.get_settings().effect_prediction.predict_death_particles
								? always_predictable_v
								: never_predictable_v
							;

							messages::change_particle_effect change;
							change.predictability = predictability;
							change.match_chased_subject = old_lying_corpse_id;
							change.match_effect_id = head_effect.id;
							change.new_target = typed_entity;
							change.new_offset = { vec2(corpse_head_offset.pos), corpse_head_offset.rotation + 180.f };

							step.post_message(change);
						}
					}
				);
			}
		}
	}
}

void handle_corpse_detonation(
	const allocate_new_entity_access access,
	const logic_step step,
	const entity_handle subject,
	components::sentience& sentience,
	const invariants::sentience& sentience_def
) {
	if (sentience.has_exploded) {
		return;
	}

	const auto& cosm = subject.get_cosmos();
	const auto& clk = cosm.get_clock();

	const auto& health = sentience.get<health_meter_instance>();
	/* The accumulated negative damage on the corpse (health.value is negative when dead) */
	const auto accumulated_corpse_damage = -health.value;

	/*
		gentle = false: violent explosion with detonation, splatters, and damage push.
		gentle = true: quiet bleed-out fall, only spawns the lying corpse entity.
	*/
	auto spawn_lying_corpse = [&](bool gentle) {
		const auto subject_pos = subject.get_logic_transform().pos;
		const auto subject_transform = subject.get_logic_transform();

		if (!gentle) {
			::detonate({
				step,
				subject,
				sentience_def.corpse_explosion,
				subject.get_logic_transform()
			}, false);

			/* Spawn blood splatters in all directions based on accumulated corpse damage */
			::spawn_blood_splatters_omnidirectional(access, step, subject, subject_pos, accumulated_corpse_damage);
		}

		/*
			Determine lying corpse rotation from last received damage direction.
			The corpse lies along the damage direction (legs point towards damage source).
		*/
		const auto lying_rotation = [&]() {
			if (sentience.last_corpse_damage_direction.is_nonzero()) {
				return (-sentience.last_corpse_damage_direction).degrees();
			}
			return subject_transform.rotation;
		}();

		const auto lying_transform = transformr(subject_pos, lying_rotation);

		/*
			Set up pending gore splatter counts regardless of gentle/violent detonation.
			These will be spawned later in sentience_system.cpp when the lying corpse velocity drops below 100.
		*/
		{
			const bool head_detached = sentience.detached.head.is_set() || sentience.knockout_origin.circumstances.headshot;
			const int num_arms_for_gore = sentience.num_arms_detached();

			if (head_detached) {
				sentience.pending_lying_gore_head = 5;
			}

			if (num_arms_for_gore >= 1) {
				sentience.pending_lying_gore_shoulder = 3;
			}

			if (num_arms_for_gore >= 2) {
				sentience.pending_lying_gore_secondary_shoulder = 3;
			}

			if (!head_detached && num_arms_for_gore == 0) {
				sentience.pending_lying_gore_center = 3;
			}
		}

		/* Choose the lying corpse flavour based on how many arms are detached */
		const auto num_arms = sentience.num_arms_detached();
		const auto corpse_flavour = [&]() {
			if (num_arms >= 2) {
				return sentience_def.lying_corpse_noarms_flavour;
			}
			if (num_arms >= 1) {
				return sentience_def.lying_corpse_noarm_flavour;
			}
			return sentience_def.lying_corpse_flavour;
		}();

		if (corpse_flavour.is_set()) {
			const auto typed_subject_id = subject.get_id();

			/*
				Flip logic:
				- Exactly 1 arm detached: use tattered flip (mirrors the standing corpse).
				- 0 or 2 arms detached: random 50/50 based on when_knocked_out.step for variety.
			*/
			const bool should_flip = [&]() {
				if (num_arms == 1) {
					return sentience.should_flip_tattered_sprite();
				}
				return sentience.when_knocked_out.step % 2 == 0;
			}();

			/*
				Lying corpse inherits velocity from the tattered corpse.
				On violent explosion, also add a push in the last damage direction.
			*/
			const auto tattered_velocity = subject.get_effective_velocity();
			const auto damage_push = [&]() {
				if (sentience.last_corpse_damage_direction.is_nonzero()) {
					return sentience.last_corpse_damage_direction.normalize() * 1300.f;
				}
				return vec2::zero;
			}();
			const auto lying_velocity = tattered_velocity + damage_push;

			/*
				Precompute the head offset for the lying corpse sprite
				so we can retarget the detached_head_particles stream.
			*/
			const bool head_detached = sentience.detached.head.is_set() || sentience.knockout_origin.circumstances.headshot;
			const auto head_effect = sentience_def.detached_head_particles;

			const auto corpse_head_offset = [&]() {
				if (head_detached && head_effect.id.is_set()) {
					const auto corpse_image_id = cosm.on_flavour(
						corpse_flavour,
						[](const auto& f) { return f.get_image_id(); }
					);

					if (corpse_image_id.is_set()) {
						auto head_off = cosm.get_logical_assets().get_offsets(corpse_image_id).torso.head;

						if (should_flip) {
							head_off.flip_vertically();
						}

						return head_off;
					}
				}

				return transformi();
			}();

			cosmic::queue_create_entity(
				step,
				corpse_flavour,
				[lying_transform, lying_velocity, should_flip, typed_subject_id](const auto& typed_entity, auto& agg) {
					typed_entity.set_logic_transform(lying_transform);

					const auto& rigid_body = typed_entity.template get<components::rigid_body>();
					rigid_body.set_velocity(lying_velocity);
					rigid_body.set_angular_velocity(0.f);

					if (should_flip) {
						if (auto* geo = agg.template find<components::overridden_geo>()) {
							geo->flip.vertically = true;
						}
					}

					if (auto* owner_comp = agg.template find<components::owner>()) {
						owner_comp->owner_body = typed_subject_id;
					}
				},

				[typed_subject_id, gentle, head_detached, head_effect, corpse_head_offset](const auto& typed_entity, const logic_step step) {
					if (const auto typed_subject = step.get_cosmos()[typed_subject_id]) {
						auto& s = typed_subject.template get<components::sentience>();
						s.detached.lying_corpse = typed_entity;
					}

					/*
						Send a white highlight for the lying corpse when it first appears.
						Analogous to the damage highlight in audiovisual_state.cpp.
					*/

					{
						constexpr float highlight_size_bounce_mult = 1.85f;

						messages::pure_color_highlight msg;
						msg.subject = typed_entity;
						msg.input.starting_alpha_ratio = 1.f;
						msg.input.color = white;

						if (!gentle) {
							msg.input.size_mult_start = highlight_size_bounce_mult;
							msg.input.maximum_duration_seconds = 0.35f;
						}

						if (gentle) {
							msg.input.size_mult_start = 1.15f;
							msg.input.maximum_duration_seconds = 0.25f;
						}

						step.post_message(msg);
					}

					/*
						Retarget the detached_head_particles stream from the tattered corpse
						to the lying corpse with the correct head offset.
					*/
					if (head_detached && head_effect.id.is_set()) {
						const auto predictability =
							step.get_settings().effect_prediction.predict_death_particles
							? always_predictable_v
							: never_predictable_v
						;

						messages::change_particle_effect change;
						change.predictability = predictability;
						change.match_chased_subject = typed_subject_id;
						change.match_orbit_offset = vec2::zero;
						change.match_effect_id = head_effect.id;
						change.new_target = typed_entity;
						change.new_offset = { vec2(corpse_head_offset.pos), corpse_head_offset.rotation + 180.f };

						step.post_message(change);
					}
				}
			);
		}

		sentience.has_exploded = true;

		/* Make the tattered corpse invisible by reinferring colliders (becomes sensor) */
		subject.infer_colliders_from_scratch();
	};

	const auto& when_ignited = sentience.when_corpse_catched_fire;
	const auto damage_past_breaking_point = accumulated_corpse_damage - sentience_def.damage_required_for_corpse_explosion;
	const bool is_ignited = when_ignited.was_set();

	if (is_ignited) {
		const auto secs_simulated_by_damaging = (damage_past_breaking_point * 2) / 1000.f;
		const auto passed_secs = clk.get_passed_secs(when_ignited);

		if (passed_secs + secs_simulated_by_damaging >= sentience_def.corpse_burning_seconds) {
			spawn_lying_corpse(false);
		}
	}
	else if (sentience.is_dead()) {
		const auto max_drips = sentience.knockout_origin.circumstances.headshot
			? IDLE_SPLATTER_MAX_CORPSE_DRIPS / 2
			: IDLE_SPLATTER_MAX_CORPSE_DRIPS
		;

		if (sentience.idle_blood_drip_count >= max_drips) {
			/*
				The tattered corpse has bled out.
				Gently fall to the ground without any explosion effects.
			*/
			spawn_lying_corpse(true);
		}
	}
}

void perform_knockout(
	const entity_id& subject_id, 
	const logic_step step, 
	const vec2 direction,
	const damage_origin& origin,
	const vec2 point_of_impact,
	const real32 damage_amount
) {
	auto& cosm = step.get_cosmos(); 

	const auto subject = cosm[subject_id];

	if (subject.dead()) {
		return;
	}

	subject.dispatch_on_having_all<invariants::sentience>([&](const auto& typed_subject) {
		auto& sentience = typed_subject.template get<components::sentience>();
		auto& sentience_def = typed_subject.template get<invariants::sentience>();
		
		if (typed_subject.template get<components::item_slot_transfers>().allow_drop_and_pick) {
			if (const auto* const container = typed_subject.template find<invariants::container>()) {
				drop_from_all_slots(*container, entity_handle(typed_subject), sentience_def.drop_impulse_on_knockout, step);
			}
		}
		else {
			::queue_delete_all_owned_items(step, typed_subject);
		}

		if (const auto* const driver = typed_subject.template find<components::driver>();
			driver != nullptr && cosm[driver->owned_vehicle].alive()
		) {
			driver_system().release_car_ownership(typed_subject);
		}

		impulse_input knockout_impulse;
		knockout_impulse.linear = direction;
		knockout_impulse.angular = 1.f;

		const auto knocked_out_body = typed_subject.template get<components::rigid_body>();
		knocked_out_body.apply(knockout_impulse * sentience_def.knockout_impulse);

		{
			auto& special_physics = typed_subject.get_special_physics();

			const auto disable_collision_for_ms = 300;

			special_physics.dropped_or_created_cooldown.set(
				disable_collision_for_ms,
				cosm.get_timestamp()
			);

			special_physics.during_cooldown_ignore_collision_with = origin.sender.capability_of_sender;
		}

		sentience.when_knocked_out = cosm.get_timestamp();
		sentience.knockout_origin = origin;

		if (sentience.is_dead()) {
			sentience.health_value_at_death = sentience.template get<health_meter_instance>().value;

			if (direction.is_nonzero()) {
				sentience.last_corpse_damage_direction = direction;
			}
		}

		if (sentience.is_dead() && origin.circumstances.headshot) {
			const auto head_transform = typed_subject.get_logic_transform();
			/*
				Head flies in the opposite direction of the damage,
				at reduced speed for a heavier feel.
				Damage-based speed scaling: 0 damage = 0%, 100 damage = 100% of base speed.
			*/
			const auto damage_ratio = std::min(1.0f, damage_amount / 100.0f);
			const auto head_velocity = -direction * (sentience_def.base_detached_head_speed * 0.5f) * damage_ratio;
			const auto typed_subject_id = typed_subject.get_id();
			const auto head_effect = sentience_def.detached_head_particles;

			/*
				Precompute the splatter origin and flight direction for the immediate head splatter.
				This will be spawned in the post_construction callback using the head entity as orbit subject.
			*/
			const auto head_splatter_origin = point_of_impact.is_nonzero() ? point_of_impact : head_transform.pos;
			const auto head_flight_dir = head_velocity.is_nonzero() ? vec2(head_velocity).normalize() : -direction;
			const auto head_access = allocate_new_entity_access();

			auto spawn_detached_body_part = [&](const auto& flavour) {
				cosmic::queue_create_entity(
					step,
					flavour,
					[head_transform, head_velocity, typed_subject_id](const auto& typed_entity, auto&) {
						typed_entity.set_logic_transform(head_transform);

						const auto& rigid_body = typed_entity.template get<components::rigid_body>();

						rigid_body.set_velocity(head_velocity);
						rigid_body.set_angular_velocity(7200.f);
						rigid_body.get_special().during_cooldown_ignore_collision_with = typed_subject_id;
					},

					[head_effect, typed_subject_id, head_access, head_splatter_origin, head_flight_dir](const auto& typed_entity, const logic_step step) {
						if (const auto typed_subject = step.get_cosmos()[typed_subject_id]) {
							auto& s = typed_subject.template get<components::sentience>();
							s.detached.head = typed_entity;
							s.pending_head_splatters = 2;
						}

						const auto predictability = 
							step.get_settings().effect_prediction.predict_death_particles 
							? always_predictable_v
							: never_predictable_v
						;

						head_effect.start(
							step,
							particle_effect_start_input::orbit_local(typed_entity, { vec2::zero, 180 } ),
							predictability
						);

						head_effect.start(
							step,
							particle_effect_start_input::orbit_local(typed_subject_id, { vec2::zero, 180 } ),
							predictability
						);

						/*
							Send a white highlight for the detached head when it first appears.
							Analogous to the lying corpse highlight.
						*/
						{
							messages::pure_color_highlight msg;
							msg.subject = typed_entity;
							msg.input.starting_alpha_ratio = 1.f;
							msg.input.maximum_duration_seconds = 0.85f;
							msg.input.size_mult_start = 3.0f;
							msg.input.color = white;

							step.post_message(msg);
						}

						/*
							Spawn an immediate splatter at the damage location,
							oriented in the head's flight direction.
							Use typed_entity (head) as orbit subject to avoid orbit glitches.

							Kept last because spawn_blood_splatter allocates an entity in the
							decal_decoration pool. If the head flavour ever shared that pool,
							typed_entity's cached pointer would be invalidated by this allocation.
						*/
						{
							auto& cosm = step.get_cosmos();
							auto rng = cosm.get_rng_for(typed_entity.get_id());
							::spawn_blood_splatter(head_access, rng, step, typed_entity, head_splatter_origin + head_flight_dir * 20.f, head_splatter_origin, 1.2f);
						}
					}
				);
			};

			spawn_detached_body_part(sentience_def.detached_flavours.head);
		}

		if (sentience.is_dead()) {
			try_detach_arms(allocate_new_entity_access(), step, typed_subject, sentience, sentience_def, point_of_impact, damage_amount, origin);
		}
	});
}
