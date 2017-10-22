#include "view/audiovisual_state/world_camera.h"
#include "augs/misc/timing/delta.h"
#include "game/transcendental/entity_handle.h"
#include "game/transcendental/cosmos.h"
#include "game/components/rigid_body_component.h"
#include "game/components/crosshair_component.h"
#include "game/components/interpolation_component.h"
#include "game/components/fixtures_component.h"
#include "view/audiovisual_state/systems/interpolation_system.h"

void world_camera::tick(
	const vec2i screen_size,
	const interpolation_system& interp,
	const augs::delta dt,
	world_camera_settings settings,
	const const_entity_handle entity_to_chase
) {
	if (const bool minimized = screen_size.is_zero()) {
		return;
	}

	const auto& cosm = entity_to_chase.get_cosmos();

	const auto enable_smoothing = settings.enable_smoothing;
	const auto angled_look_length = settings.angled_look_length;
	const auto average_factor = settings.smoothing.average_factor;
	const auto averages_per_sec = settings.smoothing.averages_per_sec;
	
	const auto target_cone = [&]() {
		auto cone = camera_cone();

		cone.visible_world_area = screen_size;

		/* we obtain transform as a copy because we'll be now offsetting it by crosshair position */
		if (entity_to_chase.alive()) {
			cone.transform = entity_to_chase.get_viewing_transform(interp);
			cone.transform.rotation = 0;
		}

		cone.transform.pos = vec2i(cone.transform.pos);

		return cone;
	}();

	const vec2i camera_crosshair_offset = get_camera_offset_due_to_character_crosshair(entity_to_chase, settings);

	current_cone = target_cone;
	current_cone.transform.pos += camera_crosshair_offset;

	if (enable_smoothing) {
		/* variable time step target_cone smoothing by averaging last position with the current */
		float averaging_constant = 1.0f - static_cast<float>(pow(average_factor, averages_per_sec * dt.in_seconds()));

		if (dont_smooth_once)
			averaging_constant = 0.0f;

		//if ((transform.pos - last_interpolant).length() < 2.0) last_interpolant = transform.pos;
		//else

		const vec2i target_pos = target_cone.transform.pos + camera_crosshair_offset;
		const vec2i smoothed_part = camera_crosshair_offset;

		last_interpolant.pos = augs::interp(vec2d(last_interpolant.pos), vec2d(smoothed_part), averaging_constant);
		last_interpolant.rotation = augs::interp(last_interpolant.rotation, target_cone.transform.rotation, averaging_constant);

		last_ortho_interpolant.x = target_cone.visible_world_area.x; //augs::interp(last_ortho_interpolant.x, visible_world_area.x, averaging_constant);
		last_ortho_interpolant.y = target_cone.visible_world_area.y; //augs::interp(last_ortho_interpolant.y, visible_world_area.y, averaging_constant);

																	   /* save smoothing result */
																	   //if ((smoothed_camera_transform.pos - last_interpolant.pos).length() > 5)
		const vec2 calculated_smoothed_pos = static_cast<vec2>(target_pos - smoothed_part) + last_interpolant.pos;
		const int calculated_smoothed_rotation = static_cast<int>(last_interpolant.rotation);

		//if (vec2i(calculated_smoothed_pos) == vec2i(smoothed_camera_transform.pos))
		//	last_interpolant.pos = smoothed_part;
		//if (int(calculated_smoothed_rotation) == int(smoothed_camera_transform.rotation))
		//	last_interpolant.rotation = smoothed_camera_transform.rotation;

		if (calculated_smoothed_pos.compare_abs(current_cone.transform.pos, 1.f)) {
			last_interpolant.pos = smoothed_part;
		}
		if (std::abs(calculated_smoothed_rotation - current_cone.transform.rotation) < 1.f) {
			last_interpolant.rotation = current_cone.transform.rotation;
		}

		current_cone.transform.pos = calculated_smoothed_pos;
		current_cone.transform.rotation = static_cast<float>(calculated_smoothed_rotation);

		//additional_position_smoothing

		current_cone.visible_world_area = last_ortho_interpolant;
	}

	if (entity_to_chase.alive()) {
		vec2 target_value;

		if (entity_to_chase.has<components::rigid_body>()) {
			auto& rigid_body = entity_to_chase.get<components::rigid_body>();

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

			// LOG("%x, %x, %x", *(vec2i*)&player_pos, *(vec2i*)&player_position_at_previous_step, *(vec2i*)&target_value);
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
		current_cone.transform.pos = vec2i(current_cone.transform.pos + additional_position_smoothing.value);
	}
	else {
		current_cone.transform.pos = vec2i(current_cone.transform.pos);
	}

	current_cone.visible_world_area = vec2i(current_cone.visible_world_area);

	dont_smooth_once = false;
}

vec2i world_camera::get_camera_offset_due_to_character_crosshair(
	const const_entity_handle entity_to_chase,
	const world_camera_settings settings
) const {
	vec2 camera_crosshair_offset;

	if (entity_to_chase.dead()) {
		return { 0, 0 };
	}

	if (const auto crosshair_entity = entity_to_chase[child_entity_name::CHARACTER_CROSSHAIR]) {
		if (const auto maybe_crosshair = crosshair_entity.find<components::crosshair>()) {
			auto& crosshair = *maybe_crosshair;

			if (crosshair.orbit_mode != components::crosshair::NONE) {
				camera_crosshair_offset = components::crosshair::calculate_aiming_displacement(crosshair_entity, false);

				if (crosshair.orbit_mode == crosshair.ANGLED) {
					camera_crosshair_offset.set_length(settings.angled_look_length);
				}

				if (crosshair.orbit_mode == crosshair.LOOK) {
					/* simple proportion */
					camera_crosshair_offset /= crosshair.base_offset_bound;
					camera_crosshair_offset *= current_cone.visible_world_area * settings.look_bound_expand;
				}
			}
		}
	}

	return camera_crosshair_offset;
}