#include <cmath>
#include "augs/templates/hash_templates.h"
#include "game/detail/sentience/sentience_logic.h"
#include "game/detail/sentience/corpse_limbs.h"
#include "game/detail/sentience/death_drop_falls.h"
#include "game/detail/decals/character_holes_layout.h"
#include "game/cosmos/cosmos.h"
#include "game/cosmos/entity_handle.h"
#include "game/stateless_systems/driver_system.h"
#include "game/detail/inventory/drop_from_all_slots.h"
#include "game/cosmos/create_entity.hpp"
#include "game/cosmos/data_living_one_step.h"
#include "game/detail/explosive/detonate.h"
#include "game/detail/sentience/gore/blood_splatter.hpp"
#include "game/detail/sentience/gore/idle_splatter.hpp"
#include "game/detail/sentience/gore/lying_corpse_stump_splatters.h"
#include "game/detail/physics/corpse_physics.h"
#include "game/messages/pure_color_highlight_message.h"
#include "game/messages/start_particle_effect.h"
#include "game/detail/view_input/sound_effect_input.h"
#include "game/components/owner_component.h"

#include "augs/log.h"

/*
	See character_gunshot_decal.h.
*/

void delete_holes_of_resurrected(const logic_step step, const entity_id character) {
	::delete_holes_attached_to(step, character);
}

static void try_detach_limbs(
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

	const auto& health = sentience.get<health_meter_instance>();
	const auto limbs_should_be_detached = ::calc_limbs_that_should_be_detached(sentience, health.value);

	if (limbs_should_be_detached <= sentience.num_limbs_detached()) {
		return;
	}

	if (!sentience_def.detached_flavours.arm_upper.is_set()) {
		return;
	}

	const auto& cosm = subject.get_cosmos();
	const auto subject_transform = subject.get_logic_transform();

	/*
		Once lying, the hit is placed on the lying corpse - the dead character stays where it stood.
	*/
	const auto lying_corpse = sentience.has_exploded ? cosm[sentience.detached.lying_corpse] : cosm[entity_id()];

	const auto hit_limb = [&]() {
		if (lying_corpse && point_of_impact.is_nonzero()) {
			return ::calc_hit_limb_lying(lying_corpse.get_logic_transform(), point_of_impact);
		}

		return ::calc_hit_limb_standing(subject_transform, subject.get_logical_size().y / 2, point_of_impact);
	}();

	/*
		One limb per hit, never two. Legs only for a character with detached leg flavours.
	*/
	const bool has_legs = sentience_def.detached_flavours.leg_upper.is_set() && sentience_def.detached_flavours.leg_lower.is_set();
	const auto chosen = ::choose_limb_to_detach(sentience, hit_limb, has_legs);

	if (!chosen.has_value()) {
		return;
	}

	const bool is_leg = chosen->is_leg;
	const bool is_upper = chosen->is_upper;

	const auto limb_flavour = [&]() {
		const auto& f = sentience_def.detached_flavours;

		if (is_leg) {
			return is_upper ? f.leg_lower : f.leg_upper;
		}

		return is_upper ? f.arm_lower : f.arm_upper;
	}();

	/*
		Upper limbs always fly up and lower limbs always fly down
		relative to the character's orientation, regardless of which side the bullet comes from.
	*/
	const auto body_rotation = lying_corpse ? lying_corpse.get_logic_transform().rotation : subject_transform.rotation;
	const auto facing_up = -vec2::from_degrees(body_rotation).perpendicular_cw();
	const auto fly_direction = is_upper ? facing_up : -facing_up;

	/* Damage-based speed scaling: 0 damage = 0%, 100 damage = 100% of base speed */
	const auto damage_ratio = repro::sqrt(std::min(1.0f, damage_amount / 100.0f));
	const auto arm_velocity = fly_direction * (sentience_def.base_detached_arm_speed * damage_ratio);
	auto arm_transform = subject_transform;
	arm_transform.pos = point_of_impact.is_nonzero() ? point_of_impact : subject_transform.pos;

	const auto typed_subject_id = subject.get_id();
	const auto head_effect = sentience_def.detached_head_particles;

	/*
		Precompute the splatter origin for the immediate arm splatter.
		This will be spawned in the post_construction callback using the arm entity as orbit subject.
	*/
	const auto arm_splatter_origin = point_of_impact.is_nonzero() ? point_of_impact : subject_transform.pos;
	const auto arm_detach_sound = sentience_def.arm_detach_sound;

	/* Increment immediately to prevent duplicate spawns before post_construction fires */
	if (is_leg) {
		(is_upper ? sentience.leg_upper_queued_for_detach : sentience.leg_lower_queued_for_detach) = true;
	}
	else {
		if (sentience.arms_queued_for_detach == 0) {
			sentience.first_arm_queued_as_upper = is_upper;
		}

		sentience.arms_queued_for_detach++;
	}
	
	if (::is_gore_award_deserved(subject, origin)) {
		sentience.coins_on_body += 500;
	}

	cosmic::queue_create_entity(
		step,
		limb_flavour,
		[arm_transform, arm_velocity, typed_subject_id](const auto& typed_entity, auto&) {
			typed_entity.set_logic_transform(arm_transform);

			const auto& rigid_body = typed_entity.template get<components::rigid_body>();
			rigid_body.set_velocity(arm_velocity);
			rigid_body.set_angular_velocity(700.f);
			rigid_body.get_special().during_cooldown_ignore_collision_with = typed_subject_id;
		},

		[head_effect, typed_subject_id, is_leg, is_upper, access, arm_splatter_origin, fly_direction, arm_detach_sound](const auto& typed_entity, const logic_step step) {
			if (const auto typed_subject = step.get_cosmos()[typed_subject_id]) {
				auto& s = typed_subject.template get<components::sentience>();
				auto& d = s.detached;

				if (is_leg) {
					(is_upper ? d.leg_upper : d.leg_lower) = typed_entity;
				}
				else {
					(is_upper ? d.arm_upper : d.arm_lower) = typed_entity;
				}

				/*
					Two rounds of splatters at all detached limbs, restarted by every new one.
				*/
				s.when_arms_detached = step.get_cosmos().get_timestamp();
				s.pending_arm_splatters = 2;
			}

			const auto predictability = ::calc_death_effect_predictability(step.get_settings().effect_prediction.death_particles);

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
				const auto predictability = ::calc_death_effect_predictability(step.get_settings().effect_prediction.death_sounds);

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

	/*
		Health, not is_dead() - for the killing hit, when_knocked_out is only set after this.
	*/
	const bool corpse_standing = !sentience.get<health_meter_instance>().is_positive() && !sentience.has_exploded;

	if (corpse_standing) {
		::keep_corpse_standing_after_hit(cosm, sentience, origin);
	}

	const auto prev_limbs_queued = sentience.num_limbs_detached();

	/*
		Not in the step it has fallen in - its lying corpse is only queued to be created, so it couldn't be replaced
		with one without the limb: the limb would be gone from the state, yet still drawn.
	*/
	const bool lying_corpse_queued = sentience.has_exploded && cosm[sentience.detached.lying_corpse].dead();

	if (!lying_corpse_queued) {
		try_detach_limbs(access, step, subject, sentience, sentience_def, point_of_impact, damage_amount, origin);
	}

	/*
		A standing corpse catches fire only past the damage that tears off all its limbs - see corpse_limbs.h.
	*/
	const auto damage_past_breaking_point = ::calc_corpse_damage_past_limbs(sentience, sentience.get<health_meter_instance>().value) - sentience_def.damage_required_for_corpse_explosion;

	if (damage_past_breaking_point > 0 && !sentience.has_exploded && !when_ignited.was_set()) {
		ignite_corpse();

		/*
			A burning corpse always explodes.
		*/
		if (::is_gore_award_deserved(subject, origin)) {
			sentience.coins_on_body += corpse_explosion_coins_v;
		}
	}

	/*
		If a limb was just detached while the lying corpse already exists,
		replace it with the correct sprite variant.
	*/
	const auto new_arms = sentience.arms_queued_for_detach;

	if (sentience.num_limbs_detached() != prev_limbs_queued && sentience.detached.lying_corpse.is_set()) {
		sentience.when_lying_corpse_replaced = cosm.get_timestamp();

		const auto lying_corpse = cosm[sentience.detached.lying_corpse];

		if (lying_corpse) {
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

			const auto new_flavour = ::calc_lying_corpse_flavour(sentience_def, sentience, should_flip);

			if (new_flavour.is_set()) {
				const auto old_transform = lying_corpse.get_logic_transform();
				const auto old_velocity = lying_corpse.get_effective_velocity();

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

					[typed_subject_id, old_lying_corpse_id, head_detached, head_effect, corpse_head_offset, mirrored = should_flip != old_flip.vertically, layout = sentience_def.corpse_layout](const auto& typed_entity, const logic_step step) {
						if (const auto typed_subject = step.get_cosmos()[typed_subject_id]) {
							auto& s = typed_subject.template get<components::sentience>();
							s.detached.lying_corpse = typed_entity;
							::start_lying_corpse_stump_splatters(s);
						}

						::move_character_decals_onto(step, old_lying_corpse_id, typed_entity, layout, false, mirrored);

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
							const auto predictability = ::calc_death_effect_predictability(step.get_settings().effect_prediction.death_particles);

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

		/* Choose the lying corpse flavour based on how many arms are detached */
		const auto num_arms = sentience.num_arms_detached();

		/*
			Flip logic:
			- Exactly 1 arm detached: use tattered flip (mirrors the standing corpse).
			- 0 or 2 arms detached: 50/50 for variety - from the character and the direction it was killed in,
			  not the step of its death: a kill that drops the corpse at once is predicted, and the server may place it
			  a step off, which would mirror the corpse after the correction.
		*/
		const bool should_flip = [&]() {
			if (num_arms == 1) {
				return sentience.should_flip_tattered_sprite();
			}

			const auto death_degrees = static_cast<int>(std::round(sentience.last_corpse_damage_direction.degrees()));
			return augs::hash_multiple(subject.get_id().raw.indirection_index, death_degrees) % 2 == 0;
		}();

		const auto corpse_flavour = ::calc_lying_corpse_flavour(sentience_def, sentience, should_flip);

		if (corpse_flavour.is_set()) {
			const auto typed_subject_id = subject.get_id();

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

					/*
						From now on it slides only head first, the way it lies - see corpse_physics.h.
					*/
					const auto& rigid_body = typed_entity.template get<components::rigid_body>();
					rigid_body.set_velocity(::project_onto_motion_axis(lying_velocity, ::calc_lying_corpse_motion_axis(lying_transform)));
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

				[typed_subject_id, gentle, head_detached, head_effect, corpse_head_offset, layout = sentience_def.corpse_layout](const auto& typed_entity, const logic_step step) {
					if (const auto typed_subject = step.get_cosmos()[typed_subject_id]) {
						auto& s = typed_subject.template get<components::sentience>();
						s.detached.lying_corpse = typed_entity;
						::start_lying_corpse_stump_splatters(s);
					}

					::move_character_decals_onto(step, typed_subject_id, typed_entity, layout, true);

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
						const auto predictability = ::calc_death_effect_predictability(step.get_settings().effect_prediction.death_particles);

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
	const auto damage_past_breaking_point = ::calc_corpse_damage_past_limbs(sentience, health.value) - sentience_def.damage_required_for_corpse_explosion;
	const bool is_ignited = when_ignited.was_set();

	/*
		Dropped at once (snipers and heavy pistols) or no longer kept up by hits - see corpse_limbs.h.
		If already burning (e.g. after a sniper headshot), it explodes right away.
	*/
	if (sentience.is_dead() && (sentience.corpse_fell_at_once || ::has_standing_corpse_time_run_out(cosm, sentience))) {
		const bool gentle = !is_ignited;
		spawn_lying_corpse(gentle);
	}
	else if (is_ignited) {
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

		/*
			Not while gun hits keep it standing - see corpse_limbs.h.
		*/
		const bool kept_standing_by_hits = sentience.corpse_stands_until.was_set();

		if (!kept_standing_by_hits && sentience.idle_blood_drip_count >= max_drips) {
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
		
		const auto held_items = typed_subject.get_wielded_items();

		if (typed_subject.template get<components::item_slot_transfers>().allow_drop_and_pick) {
			if (const auto* const container = typed_subject.template find<invariants::container>()) {
				drop_from_all_slots(*container, entity_handle(typed_subject), sentience_def.drop_distance_on_knockout, step);
			}

			if (!sentience.template get<health_meter_instance>().is_positive()) {
				::start_dropped_on_death_falls(step, held_items);
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

				/*
					The standing corpse slides on only forward along the direction it was killed in -
					still free to turn - see corpse_physics.h.
				*/
				::constrain_to_motion_axis(typed_subject);
			}

			::reveal_holes_of_the_dead(step, typed_subject);
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

						const auto predictability = ::calc_death_effect_predictability(step.get_settings().effect_prediction.death_particles);

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
			try_detach_limbs(allocate_new_entity_access(), step, typed_subject, sentience, sentience_def, point_of_impact, damage_amount, origin);
		}
	});
}
