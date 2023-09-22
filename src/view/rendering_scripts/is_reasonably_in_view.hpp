#pragma once

template <class E, class F>
bool is_reasonably_in_view(
	const E viewed_character,
	const F target_character,
	const vec2 pre_step_crosshair_displacement,
	const interpolation_system& interp,
	const float fow_angle,
	const bool teammates_are_enemies
) {
	const auto viewed_character_transform = viewed_character ? viewed_character.find_viewing_transform(interp) : std::optional<transformr>();

	if (viewed_character.dead() || viewed_character_transform == std::nullopt) {
		return false;
	}

	if (viewed_character == target_character) {
		return true;
	}

	if (!teammates_are_enemies && viewed_character.get_official_faction() == target_character.get_official_faction()) {
		return true;
	}

	const auto& cosm = viewed_character.get_cosmos();

	const auto from = viewed_character_transform->pos;
	const auto to = target_character.get_viewing_transform(interp).pos;

	auto look_dir = calc_crosshair_displacement(viewed_character) + pre_step_crosshair_displacement;

	if (look_dir.is_zero()) {
		look_dir.set(1, 0);
	}

	look_dir.normalize();
	const auto target_dir = (to - from).normalize();

	if (look_dir.degrees_between(target_dir) <= fow_angle / 2) {
		const auto& physics = cosm.get_solvable_inferred().physics;

		const auto line_of_sight = physics.ray_cast_px(
			cosm.get_si(), 
			from, 
			to, 
			predefined_queries::line_of_sight()
		);

		return !line_of_sight.hit;
	}

	return false;
}

