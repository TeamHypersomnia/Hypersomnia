#include "view/audiovisual_state/world_camera.h"
#include "augs/misc/timing/delta.h"
#include "game/cosmos/entity_handle.h"
#include "game/cosmos/cosmos.h"
#include "game/components/rigid_body_component.h"
#include "game/components/crosshair_component.h"
#include "game/components/interpolation_component.h"
#include "game/components/fixtures_component.h"
#include "view/audiovisual_state/systems/interpolation_system.h"
#include "game/detail/crosshair_math.hpp"

void world_camera::tick(
	const vec2i screen_size,
	const interpolation_system& interp,
	const augs::delta dt,
	world_camera_settings settings,
	const const_entity_handle entity_to_chase
) {
	if (/* minimized */ screen_size.is_zero()) {
		return;
	}

	const auto& cosm = entity_to_chase.get_cosmos();

	const auto enable_smoothing = settings.enable_smoothing;
	const auto average_factor = settings.smoothing.average_factor;
	const auto averages_per_sec = settings.smoothing.averages_per_sec;
	
	const auto target_cone = [&]() {
		/* we obtain transform as a copy because we'll be now offsetting it by crosshair position */
		if (entity_to_chase.alive()) {
			auto cone = camera_eye();

			cone.transform = entity_to_chase.get_viewing_transform(interp);
			cone.transform.rotation = 0;

			return cone;
		}

		return current_eye;
	}();

	const vec2 camera_crosshair_offset = get_camera_offset_due_to_character_crosshair(entity_to_chase, settings, screen_size);

	current_eye = target_cone;
	current_eye.transform.pos += camera_crosshair_offset;

	if (enable_smoothing) {
		/* variable time step target_cone smoothing by averaging last position with the current */
		float averaging_constant = 1.0f - static_cast<float>(pow(average_factor, averages_per_sec * dt.in_seconds()));

		if (dont_smooth_once)
			averaging_constant = 0.0f;

		//if ((transform.pos - last_interpolant).length() < 2.0) last_interpolant = transform.pos;
		//else

		const vec2 target_pos = target_cone.transform.pos + camera_crosshair_offset;
		const vec2 smoothed_part = camera_crosshair_offset;

		last_interpolant.pos = augs::interp(vec2d(last_interpolant.pos), vec2d(smoothed_part), averaging_constant);
		last_interpolant.rotation = augs::interp(last_interpolant.rotation, target_cone.transform.rotation, averaging_constant);

																	   //if ((smoothed_camera_eye.pos - last_interpolant.pos).length() > 5)
		const vec2 calculated_smoothed_pos = static_cast<vec2>(target_pos - smoothed_part) + last_interpolant.pos;
		const int calculated_smoothed_rotation = static_cast<int>(last_interpolant.rotation);

		//if (vec2(calculated_smoothed_pos) == vec2(smoothed_camera_eye.pos))
		//	last_interpolant.pos = smoothed_part;
		//if (int(calculated_smoothed_rotation) == int(smoothed_camera_eye.rotation))
		//	last_interpolant.rotation = smoothed_camera_eye.rotation;

		if (calculated_smoothed_pos.compare_abs(current_eye.transform.pos, 1.f)) {
			last_interpolant.pos = smoothed_part;
		}
		if (std::abs(calculated_smoothed_rotation - current_eye.transform.rotation) < 1.f) {
			last_interpolant.rotation = current_eye.transform.rotation;
		}

		current_eye.transform.pos = calculated_smoothed_pos;
		current_eye.transform.rotation = static_cast<float>(calculated_smoothed_rotation);
	}

	if (entity_to_chase.alive()) {
		vec2 target_value;

		if (const auto rigid_body = entity_to_chase.find<components::rigid_body>()) {
			vec2 player_pos;

			player_pos = rigid_body.get_position();//entity_to_chase.get_logic_transform().interpolated(dt.view_interpolation_ratio());
				//rigid_body.get_position();

			if (player_pos != player_position_at_previous_step) {
				player_position_previously_seen = player_position_at_previous_step;
				player_position_at_previous_step = player_pos;
			}

			target_value = (player_pos - player_position_previously_seen) * cosm.get_fixed_delta().in_milliseconds();

			if (target_value.length() < additional_position_smoothing.value.length()) {
				// braking
				settings.additional_position_smoothing.averages_per_sec += 3.5;
			}
			else {
				settings.additional_position_smoothing.averages_per_sec += 1.5;
			}

			if (target_value.length() > 50) {
				target_value.set_length(50);
			}

			// LOG("%x, %x, %x", *(vec2*)&player_pos, *(vec2*)&player_position_at_previous_step, *(vec2*)&target_value);
		}
		//else {
		//	target_value = chased_transform.interpolation_direction(previous);
		//	target_value.set_length(100);
		//	additional_position_smoothing.averages_per_sec = 5;
		//}

		additional_position_smoothing.target_value = target_value * (-1);
		additional_position_smoothing.tick(dt, settings.additional_position_smoothing);
	}

	if (enable_smoothing) {
		current_eye.transform.pos = vec2(current_eye.transform.pos + additional_position_smoothing.value);
	}
	else {
		current_eye.transform.pos = vec2(current_eye.transform.pos);
	}

	dont_smooth_once = false;
}

vec2 world_camera::get_camera_offset_due_to_character_crosshair(
	const const_entity_handle entity_to_chase,
	const world_camera_settings settings,
	const vec2 screen_size
) const {
	vec2 camera_crosshair_offset;

	if (entity_to_chase.dead()) {
		return { 0, 0 };
	}

	if (const auto crosshair = entity_to_chase.find_crosshair()) {
		if (crosshair->orbit_mode != crosshair_orbit_type::NONE) {
			camera_crosshair_offset = calc_crosshair_displacement(entity_to_chase);

			if (crosshair->orbit_mode == crosshair_orbit_type::ANGLED) {
				camera_crosshair_offset.set_length(settings.angled_look_length);
			}

			if (crosshair->orbit_mode == crosshair_orbit_type::LOOK) {
				const auto bound = screen_size;

				if (bound.neither_zero()) {
					camera_crosshair_offset /= bound;
				}

				camera_crosshair_offset *= camera_cone(current_eye, screen_size).get_visible_world_area() * settings.look_bound_expand;
			}
		}
	}

	return camera_crosshair_offset;
}