#pragma once

template <class A, class B>
static void ricochet_missile_against_surface(
	const logic_step step,
	const A& typed_missile,
	const B& surface_handle,

	const vec2& normal,
	const vec2& collider_impact_velocity,
	const vec2& point
) {
	auto& cosm = step.get_cosmos();
	const auto& clk = cosm.get_clock();
	const auto& now = clk.now;

	RIC_LOG("(RIC)(%x) MISSILE %x WITH: %x", now.step, "CONTACT START", surface_handle);

	const auto& missile_def = typed_missile.template get<invariants::missile>();
	const auto ricochet_cooldown_ms = missile_def.ricochet_cooldown_ms;
	(void)missile_def;
	(void)ricochet_cooldown_ms;

	auto& missile = typed_missile.template get<components::missile>();

	const auto info = missile_surface_info(typed_missile, surface_handle);

	if (missile.during_penetration) {
		return;
	}

	if (!info.is_ricochetable()) {
		RIC_LOG("non-ricochetable surface, IGNORED");
		return;
	}

	const auto collision_normal = vec2(normal).normalize();

	const auto impact_velocity = collider_impact_velocity;
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

	const auto& surface_fixtures = surface_handle.template get<invariants::fixtures>();
	const auto max_ricochet_angle = surface_fixtures.max_ricochet_angle;

	const auto left_b = 90 - max_ricochet_angle;
	const auto right_b = 90 + max_ricochet_angle;

	if (DEBUG_DRAWING.draw_forces) {
		DEBUG_PERSISTENT_LINES.emplace_back(
			yellow,
			vec2(point),
			vec2(point) + collision_normal * 150
		);

		DEBUG_PERSISTENT_LINES.emplace_back(
			green,
			vec2(point),
			vec2(point) - impact_dir * 150
		);
	}

	RIC_LOG_NVPS(left_b, hit_facing, right_b);

	if (max_ricochet_angle >= 180 || (hit_facing > left_b && hit_facing < right_b)) {
		{
			const bool born_cooldown = clk.lasts(
				missile_def.ricochet_born_cooldown_ms,
				typed_missile.when_born()
			);

			if (!surface_fixtures.point_blank_ricochets && born_cooldown) {
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

		const auto effect_transform = transformr(point, reflected_dir.degrees());
		//::play_collision_sound(angle_mult * 150.f, effect_transform, typed_missile, surface_handle, step);

		{
			const auto& effect = missile_def.ricochet_particles;

			effect.start(
				step,
				particle_effect_start_input::fire_and_forget(effect_transform),
				always_predictable_v
			);
		}

		{
			const auto pitch = 0.7f + angle_mult / 1.5f;

			auto effect = missile_def.ricochet_sound;
			effect.modifier.pitch = pitch;

			// TODO: PARAMETRIZE!
			effect.modifier.max_distance = 3000.f;
			effect.modifier.reference_distance = 1000.f;

			effect.start(
				step,
				sound_effect_start_input::fire_and_forget(effect_transform),
				always_predictable_v
			);
		}
	}
	else {
		RIC_LOG("Not enough facing. IGNORED.");
	}
}
