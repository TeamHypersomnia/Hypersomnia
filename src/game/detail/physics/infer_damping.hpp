#pragma once

template <class E>
damping_mults calc_damping_mults(const E& handle, const invariants::rigid_body& def) {
	damping_mults damping = def.damping;

	handle.template dispatch_on_having_all<components::movement>([&damping](const auto& typed_handle) {
		const auto& movement = typed_handle.template get<components::movement>();
		const auto& movement_def = typed_handle.template get<invariants::movement>();

		const bool is_inert = movement.const_inertia_ms > 0.f || movement.linear_inertia_ms > 0.f;

		if (movement.const_inertia_ms > 0.f) {
			damping.linear = 2;
		}
		else {
			damping.linear = movement_def.standard_linear_damping;
		}

		{
			const auto m = movement_def.max_linear_inertia_when_movement_possible;
			const auto inertia_mult = std::clamp(1.f - movement.linear_inertia_ms / m, 0.f, 1.f);

			damping.linear *= inertia_mult;
		}

		const auto requested_by_input = movement.get_force_requested_by_input(movement_def.input_acceleration_axes);

		if (requested_by_input.non_zero()) {
			if (movement.was_sprint_effective) {
				if (!is_inert) {
					damping.linear /= 4;
				}
			}
		}

		const bool make_inert = movement.const_inertia_ms > 0.f;

		/* the player feels less like a physical projectile if we brake per-axis */
		if (!make_inert) {
			damping.linear_axis_aligned += vec2(
				requested_by_input.x_non_zero() ? 0.f : movement_def.braking_damping,
				requested_by_input.y_non_zero() ? 0.f : movement_def.braking_damping
			);
		}
	});

	return damping.sanitize();
}

template <class E, class B>
void infer_damping(const E& handle, B& b) {
	const auto& def = handle.template get<invariants::rigid_body>();
	const auto damping = ::calc_damping_mults(handle, def);

	b.SetLinearDamping(damping.linear);
	b.SetAngularDamping(damping.angular);
	b.SetLinearDampingVec(b2Vec2(damping.linear_axis_aligned));
	b.SetAngledDampingEnabled(def.angled_damping);
}

