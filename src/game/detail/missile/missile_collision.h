#pragma once
#include "game/detail/physics/missile_surface_info.h"
#include "game/detail/sentience/sentience_getters.h"
#include "game/detail/missile/headshot_detection.hpp"

struct missile_collision_result {
	transformr transform_of_impact;
	int new_charges_value = 0;
};


enum class missile_collision_type {
	CONTACT_START,
	PRE_SOLVE,
};

template <class A, class B>
static std::optional<missile_collision_result> collide_missile_against_surface(
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

	RIC_LOG("(DET) MISSILE %x WITH: %x", pre_solve ? "PRE SOLVE" : "CONTACT START", surface_handle);

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
		&& missile.damage_charges_before_destruction > 0
	;

	if (!should_send_damage) {
		return std::nullopt;
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
			return std::nullopt;
		}

		RIC_LOG("DET: NO COOLDOWN, IMPACTING");
	}

	const auto impact_velocity = collider_impact_velocity;
	const auto impact_dir = vec2(impact_velocity).normalize();

	auto charges = missile.damage_charges_before_destruction;
	const bool send_damage = charges > 0;

	messages::damage_message damage_msg;
	damage_msg.indices = indices;
	damage_msg.damage = missile_def.damage;
	damage_msg.damage *= missile.power_multiplier_of_sender;

	const auto sentience = surface_handle.template find<invariants::sentience>();
	const bool surface_sentient = sentience != nullptr;

	if (info.should_detonate() && missile_def.destroy_upon_damage) {
		--charges;
		
		detonate_if(typed_missile.get_id(), point, step);

		{
			const auto& total_damage_amount = damage_msg.damage.base;

			if (augs::is_positive_epsilon(total_damage_amount)) {
				startle_nearby_organisms(cosm, point, total_damage_amount * 12.f, 27.f, startle_type::LIGHTER);
				startle_nearby_organisms(cosm, point, total_damage_amount * 6.f, 50.f + total_damage_amount * 2.f, startle_type::IMMEDIATE);
			}
		}

		// delete only once
		if (charges == 0) {
			step.queue_deletion_of(typed_missile, "Missile collision");
			damage_msg.inflictor_destructed = true;

			auto rng = cosm.get_nontemporal_rng_for(typed_missile);

			if (!surface_sentient) {
				spawn_bullet_remnants(
					step,
					rng,
					missile_def.remnant_flavours,
					collision_normal,
					impact_dir,
					point
				);
			}
		}
	}

	if (send_damage && contact_start) {
		if (surface_sentient) {
			const auto missile_pos = point;
			const auto head_transform = ::calc_head_transform(surface_handle);
			const auto head_radius = sentience->head_hitbox_radius;

			if (head_transform != std::nullopt) {
				const auto head_pos = head_transform->pos;

				if (DEBUG_DRAWING.draw_headshot_detection) {
					DEBUG_PERSISTENT_LINES.emplace_back(
						cyan,
						missile_pos,
						missile_pos + impact_dir * 100
					);

					DEBUG_PERSISTENT_LINES.emplace_back(
						red,
						head_pos,
						head_pos + vec2(0, head_radius)
					);

					DEBUG_PERSISTENT_LINES.emplace_back(
						red,
						head_pos,
						head_pos + vec2(head_radius, 0)
					);

					DEBUG_PERSISTENT_LINES.emplace_back(
						red,
						head_pos,
						head_pos + vec2(-head_radius, 0)
					);

					DEBUG_PERSISTENT_LINES.emplace_back(
						red,
						head_pos,
						head_pos + vec2(0, -head_radius)
					);
				}

				if (::headshot_detected(
					missile_pos,
					impact_dir,
					head_pos,
					head_radius
				)) {
					damage_msg.damage *= missile.headshot_multiplier_of_sender;
					damage_msg.headshot = true;
					damage_msg.head_transform = *head_transform;
				}
			}
		}

		damage_msg.origin = damage_origin(typed_missile);
		damage_msg.subject = surface_handle;
		damage_msg.impact_velocity = impact_velocity;
		damage_msg.point_of_impact = point;
		step.post_message(damage_msg);
	}

	return missile_collision_result { transformr { point, impact_dir.degrees() }, charges };
}
