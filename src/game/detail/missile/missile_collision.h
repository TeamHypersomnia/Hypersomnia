#pragma once
#include "game/detail/physics/missile_surface_info.h"

struct missile_collision_result {
	transformr saved_point_of_impact_before_death;
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

	const auto& impact_impulse = missile_def.damage.impact_impulse;

	if (impact_impulse > 0.f && contact_start) {
		auto considered_impulse = impact_impulse * missile.power_multiplier_of_sender;

		if (const auto sentience = surface_handle.template find<components::sentience>()) {
			const auto& shield_of_victim = sentience->template get<electric_shield_perk_instance>();

			if (!shield_of_victim.timing.is_enabled(clk)) {
				considered_impulse *= missile_def.damage.impulse_multiplier_against_sentience;
			}
		}

		const auto subject_of_impact = surface_handle.get_owner_of_colliders().template get<components::rigid_body>();
		const auto subject_of_impact_mass_pos = subject_of_impact.get_mass_position(); 

		const auto impact = vec2(impact_velocity).set_length(considered_impulse);

		subject_of_impact.apply_impulse(impact, point - subject_of_impact_mass_pos);
	}

	auto saved_point_of_impact_before_death = transformr { point, impact_dir.degrees() };

	const auto owning_capability = surface_handle.get_owning_transfer_capability();

	{
		const bool is_victim_a_held_item = 
			info.surface_is_item 
			&& owning_capability.alive() 
			&& owning_capability != surface_handle
		;

		if (is_victim_a_held_item && contact_start) {
			missile_def.damage.pass_through_held_item_sound.start(
				step,
				sound_effect_start_input::fire_and_forget( { point, 0.f } ).set_listener(owning_capability)
			);
		}
	}

	const auto total_damage_amount = 
		missile_def.damage.base * 
		missile.power_multiplier_of_sender
	;

	auto charges = missile.damage_charges_before_destruction;
	const bool send_damage = charges > 0;

	messages::damage_message damage_msg;
	damage_msg.indices = indices;
	damage_msg.effects = missile_def.damage.effects;

	if (info.should_detonate() && missile_def.destroy_upon_damage) {
		--charges;
		
		detonate_if(typed_missile.get_id(), point, step);

		if (augs::is_positive_epsilon(total_damage_amount)) {
			startle_nearby_organisms(cosm, point, total_damage_amount * 12.f, 27.f, startle_type::LIGHTER);
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
				point
			);
		}
	}

	if (send_damage && contact_start) {
		damage_msg.origin = damage_origin(typed_missile);
		damage_msg.subject = surface_handle;
		damage_msg.amount = total_damage_amount;
		damage_msg.victim_shake = missile_def.damage.shake;
		damage_msg.victim_shake *= missile.power_multiplier_of_sender;
		damage_msg.impact_velocity = impact_velocity;
		damage_msg.point_of_impact = point;
		step.post_message(damage_msg);
	}

	return missile_collision_result { saved_point_of_impact_before_death, charges };
}
