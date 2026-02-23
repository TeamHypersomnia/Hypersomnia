#pragma once
#include "game/messages/damage_message.h"
#include "game/detail/physics/missile_surface_info.h"
#include "game/detail/sentience/sentience_getters.h"
#include "game/detail/missile/headshot_detection.hpp"
#include "game/stateless_systems/sentience_system.h"

#if HEADLESS
void draw_headshot_debug_lines(vec2, vec2, vec2, float) {}
#else
void draw_headshot_debug_lines(vec2 missile_pos, vec2 impact_dir, vec2 head_pos, float head_radius);
#endif

struct missile_collision_result {
	transformr transform_of_impact;
	bool deleted_already = false;
	bool penetration_began = false;
};

enum class missile_collision_type {
	CONTACT_START,
	PRE_SOLVE,
};

template <class A, class B>
static std::optional<missile_collision_result> collide_missile_against_surface(
	allocate_new_entity_access access,
	const logic_step step,

	const A& typed_missile,
	const B& surface_handle,

	const invariants::missile& missile_def,
	const components::missile& missile,

	const missile_collision_type type,
	const missile_surface_info& info,

	const b2Fixture_indices indices,

	const vec2& normal,
	const vec2& collider_impact_velocity,
	const vec2& point
) {
	const bool contact_start  = type == missile_collision_type::CONTACT_START;
	const bool pre_solve = type == missile_collision_type::PRE_SOLVE;

	auto& cosm = step.get_cosmos();
	const auto& clk = cosm.get_clock();
	const auto& now = clk.now;

	//LOG("(DET) MISSILE %x WITH: %x", pre_solve ? "PRE SOLVE" : "CONTACT START", surface_handle);
	//LOG_NVPS(point, typed_missile.get_logic_transform().pos);

	const auto& ricochet_cooldown_ms = missile_def.ricochet_cooldown_ms;
	(void)missile_def;
	(void)ricochet_cooldown_ms;

	const auto collision_normal = vec2(normal).normalize();

	if (info.should_ignore_altogether()) {
		RIC_LOG("IGNORED");
		return std::nullopt;
	}

	const bool should_send_damage =
		missile_def.damage_upon_collision
		&& !missile.deleted_already
	;

	if (!should_send_damage) {
		return std::nullopt;
	}

	if (pre_solve) {
		/* With a PreSolve must have happened a PostSolve that altered our initial velocity. */

		/* Not sure if this is needed here actually but let it be */
		make_velocity_face_body_orientation(typed_missile);
	}

	{
		const bool missile_ricochetted_in_this_step = 
			missile.when_last_ricocheted.was_set()
			&& now.step <= missile.when_last_ricocheted.step + 1
		;

		if (missile_ricochetted_in_this_step) {
			RIC_LOG("DET: This impact counted as ricochet already.");
			return std::nullopt;
		}

		RIC_LOG("DET: NO COOLDOWN, IMPACTING");
	}

	const auto impact_velocity = collider_impact_velocity;
	const auto impact_dir = vec2(impact_velocity).normalize();

	bool penetration_began = false;
	auto deleted_already = missile.deleted_already;

	const auto sentience_def = surface_handle.template find<invariants::sentience>();
	const auto sentience = surface_handle.template find<components::sentience>();

	const bool surface_sentient = sentience_def != nullptr && sentience != nullptr;

	auto finalize_bullet = [&]() {
		if (!missile_def.destroy_upon_damage) {
			return;
		}

		deleted_already = true;

		step.queue_deletion_of(typed_missile, "Missile collision");

		auto rng = cosm.get_nontemporal_rng_for(typed_missile);

		if (!surface_sentient) {
			spawn_bullet_remnants(
				access,
				step,
				rng,
				missile_def.remnant_flavours,
				collision_normal,
				impact_dir,
				point
			);
		}
	};

	auto penetrate_or_finalize = [&]() {
		if (missile.penetration_distance_remaining > 0.0f) {
			if (!missile.during_penetration) {
				penetration_began = true;
			}
		}
		else {
			finalize_bullet();
		}
	};

	if (contact_start && !deleted_already) {
		messages::damage_message damage_msg;
		damage_msg.indices = indices;
		damage_msg.damage = missile_def.damage;
		damage_msg.damage *= missile.power_multiplier_of_sender;

		const auto dist_remaining = missile.penetration_distance_remaining;
		const auto dist_starting = missile.starting_penetration_distance;

		if (info.should_detonate()) {
			detonate_if(typed_missile.get_id(), point, step);

			{
				const auto& total_damage_amount = damage_msg.damage.base;

				if (augs::is_positive_epsilon(total_damage_amount)) {
					startle_nearby_organisms(cosm, point, total_damage_amount * 12.f, 27.f, startle_type::LIGHTER);
					startle_nearby_organisms(cosm, point, total_damage_amount * 6.f, 50.f + total_damage_amount * 2.f, startle_type::IMMEDIATE);
				}
			}
		}

		damage_msg.origin = damage_origin(typed_missile);
		damage_msg.subject = surface_handle;
		damage_msg.impact_velocity = impact_velocity;
		damage_msg.normal = collision_normal;
		damage_msg.point_of_impact = point;

		if (dist_remaining != dist_starting && dist_starting != 0.0f) {
			damage_msg.damage *= dist_remaining / dist_starting;
			damage_msg.origin.circumstances.wallbang = true;
		}

		if (surface_sentient) {
			const auto missile_entity_id = typed_missile.get_id();
			const bool is_duplicate_bullet_hit = sentience->ignore_bullet == entity_id(missile_entity_id);

			if (is_duplicate_bullet_hit) {
				/*
					This bullet already damaged this sentience entity once.
					Ignore to prevent double damage from the same bullet.
				*/
				return std::nullopt;
			}

			const auto missile_pos = point;
			const auto head_transform = ::calc_head_transform(surface_handle);
			const auto head_radius = sentience_def->head_hitbox_radius * missile.head_radius_multiplier_of_sender;

			if (head_transform.has_value()) {
				const auto head_pos = head_transform->pos;

				::draw_headshot_debug_lines(missile_pos, impact_dir, head_pos, head_radius);

				if (::headshot_detected(
					missile_pos,
					impact_dir,
					head_pos,
					head_radius
				)) {
					damage_msg.origin.circumstances.headshot = true;
					damage_msg.headshot_mult = missile.headshot_multiplier_of_sender;
					damage_msg.head_transform = *head_transform;
				}
			}

			const bool was_conscious_before = sentience->is_conscious();

			sentience_system().process_damage_message(damage_msg, step);
			damage_msg.processed = true;

			const bool is_conscious_now = sentience->is_conscious();
			const bool just_died = was_conscious_before && !is_conscious_now;

			/*
				Store this bullet in ignore_bullet so that if the bullet
				continues to exist (e.g. character died), it won't damage again.
			*/
			sentience->ignore_bullet = entity_id(missile_entity_id);

			if (just_died) {
				/*
					When character receives damage that kills him, subtract as much health
					from the corpse as if it was hit by that bullet yet another time.
					This is done discreetly without showing additional damage indicators.
				*/
				auto& health = sentience->template get<health_meter_instance>();

				const auto additional_damage = damage_msg.damage.base; // * (damage_msg.origin.circumstances.headshot ? damage_msg.headshot_mult : 1.0f);

				/* A little less after all */
				if (true) {
					health.value -= additional_damage / 2.0f;
				}
			}

			if (sentience->is_conscious()) {
				finalize_bullet();
				damage_msg.spawn_destruction_effects = true;
			}
		}
		else {
			if (!info.ignore_standard_collision_resolution()) {
				penetrate_or_finalize();
				damage_msg.spawn_destruction_effects = true;
			}
		}

		step.post_message(damage_msg);
	}

	return missile_collision_result { transformr { point, impact_dir.degrees() }, deleted_already, penetration_began };
}
