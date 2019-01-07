#pragma once

template <class H>
bool legs_frozen(const H& it) {
	const auto& movement = it.template get<components::movement>();
	const auto& movement_def = it.template get<invariants::movement>();

	return 
		movement.const_inertia_ms
		+ movement.linear_inertia_ms
		> movement_def.freeze_legs_when_inertia_exceeds
	;
}

template <class H>
bool dash_conditions_fulfilled(const H& it) {
	const auto& movement = it.template get<components::movement>();

	return movement.dash_cooldown_ms <= 0.f && !legs_frozen(it);
}
