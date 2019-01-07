#pragma once

template <class T>
auto perform_dash(
	const T& handle,
	const vec2 dash_dir,
	const real32 dash_impulse,
	const real32 make_inert_for_ms
) {
	{
		auto& movement = handle.template get<components::movement>();
		movement.linear_inertia_ms += make_inert_for_ms;
	}

	const auto& body = handle.template get<components::rigid_body>();

	const auto current_velocity = body.get_velocity();
	const auto current_speed = current_velocity.length();

	const auto vel_dir = vec2(current_velocity).normalize_hint(current_speed);
	const auto impulse_mult = (vel_dir.dot(dash_dir) + 1) / 2;

	body.apply_impulse(impulse_mult * dash_dir * body.get_mass() * dash_impulse);

	const auto& movement_def  = handle.template get<invariants::movement>();
	const auto conceptual_max_speed = std::max(1.f, movement_def.max_speed_for_animation);
	const auto speed_mult = current_speed / conceptual_max_speed;
	const auto effect_mult = speed_mult * impulse_mult;

	return effect_mult;
}
