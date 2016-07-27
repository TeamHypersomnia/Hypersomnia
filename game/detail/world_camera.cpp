#include "world_camera.h"
#include "game/transcendental/entity_handle.h"
#include "game/transcendental/cosmos.h"
#include "game/components/physics_component.h"
#include "game/components/crosshair_component.h"
#include "augs/misc/delta.h"

void world_camera::configure_size(vec2 size) {
	visible_world_area = size;
}

void world_camera::tick(augs::variable_delta dt, const_entity_handle entity_to_chase) {
	/* we obtain transform as a copy because we'll be now offsetting it by crosshair position */
	if (entity_to_chase.alive()) {
		transform = entity_to_chase.get<components::transform>().interpolated(dt.view_interpolation_ratio());
		transform.rotation = 0;
	}

	transform.pos = vec2i(transform.pos);

	vec2i camera_crosshair_offset = get_camera_offset_due_to_character_crosshair(entity_to_chase);

	smoothed_camera_transform = transform;
	smoothed_visible_world_area = visible_world_area;

	smoothed_camera_transform.pos += camera_crosshair_offset;

	if (enable_smoothing) {
		/* variable time step camera smoothing by averaging last position with the current */
		float averaging_constant = 1.0f - static_cast<float>(pow(smoothing_average_factor, averages_per_sec * dt.in_seconds()));

		if (dont_smooth_once)
			averaging_constant = 0.0f;

		//if ((transform.pos - last_interpolant).length() < 2.0) last_interpolant = transform.pos;
		//else

		vec2i target = transform.pos + camera_crosshair_offset;
		vec2i smoothed_part = camera_crosshair_offset;

		last_interpolant.pos = augs::interp(vec2d(last_interpolant.pos), vec2d(smoothed_part), averaging_constant);
		last_interpolant.rotation = augs::interp(last_interpolant.rotation, transform.rotation, averaging_constant);

		last_ortho_interpolant.x = visible_world_area.x; //augs::interp(last_ortho_interpolant.x, visible_world_area.x, averaging_constant);
		last_ortho_interpolant.y = visible_world_area.y; //augs::interp(last_ortho_interpolant.y, visible_world_area.y, averaging_constant);

																	   /* save smoothing result */
																	   //if ((smoothed_camera_transform.pos - last_interpolant.pos).length() > 5)
		vec2 calculated_smoothed_pos = static_cast<vec2>(target - smoothed_part) + last_interpolant.pos;
		int calculated_smoothed_rotation = static_cast<int>(last_interpolant.rotation);

		//if (vec2i(calculated_smoothed_pos) == vec2i(smoothed_camera_transform.pos))
		//	last_interpolant.pos = smoothed_part;
		//if (int(calculated_smoothed_rotation) == int(smoothed_camera_transform.rotation))
		//	last_interpolant.rotation = smoothed_camera_transform.rotation;

		if (calculated_smoothed_pos.compare_abs(smoothed_camera_transform.pos, 1.f))
			last_interpolant.pos = smoothed_part;
		if (std::abs(calculated_smoothed_rotation - smoothed_camera_transform.rotation) < 1.f)
			last_interpolant.rotation = smoothed_camera_transform.rotation;

		smoothed_camera_transform.pos = calculated_smoothed_pos;
		smoothed_camera_transform.rotation = static_cast<float>(calculated_smoothed_rotation);

		//smoothing_player_pos

		smoothed_visible_world_area = last_ortho_interpolant;
	}

	if (entity_to_chase.alive()) {
		vec2 target_value;

		if (entity_to_chase.has<components::physics>()) {
			auto& physics = entity_to_chase.get<components::physics>();

			vec2 player_pos;

			player_pos = physics.get_position();//entity_to_chase.get<components::transform>().interpolated(dt.view_interpolation_ratio());
				//physics.get_position();

			if (player_pos != previous_step_player_position) {
				previous_seen_player_position = previous_step_player_position;
				previous_step_player_position = player_pos;
			}

			target_value = (player_pos - previous_seen_player_position) * dt.in_milliseconds();
			auto vel = physics.velocity();

			if (target_value.length() < smoothing_player_pos.value.length())
				// braking
				smoothing_player_pos.averages_per_sec = 3.5;
			else
				smoothing_player_pos.averages_per_sec = 1.5;

			if (target_value.length() > 50)
				target_value.set_length(50);

			// LOG("%x, %x, %x", *(vec2i*)&player_pos, *(vec2i*)&previous_step_player_position, *(vec2i*)&target_value);
		}
		else {
			target_value = entity_to_chase.get<components::transform>().interpolation_direction();
			target_value.set_length(100);
			smoothing_player_pos.averages_per_sec = 5;
		}

		smoothing_player_pos.target_value = target_value * (-1);
		smoothing_player_pos.tick(dt.in_seconds());
	}

	if (enable_smoothing)
		smoothed_camera_transform.pos = vec2i(smoothed_camera_transform.pos + smoothing_player_pos.value);
	else
		smoothed_camera_transform.pos = vec2i(smoothed_camera_transform.pos);

	smoothed_visible_world_area = vec2i(smoothed_visible_world_area);

	dont_smooth_once = false;
}

state_for_drawing_camera world_camera::get_state_for_drawing_camera(const_entity_handle entity_to_chase) {
	state_for_drawing_camera in;
	in.transformed_visible_world_area_aabb = augs::get_aabb_rotated(smoothed_visible_world_area, smoothed_camera_transform.rotation)
		+ smoothed_camera_transform.pos - smoothed_visible_world_area / 2;
	in.camera_transform = smoothed_camera_transform;
	in.visible_world_area = smoothed_visible_world_area;
	in.associated_character = entity_to_chase;

	return in;
}

vec2i world_camera::get_camera_offset_due_to_character_crosshair(const_entity_handle entity_to_chase) const {
	vec2 camera_crosshair_offset;

	if (entity_to_chase.dead())
		return vec2i(0, 0);

	auto crosshair_entity = entity_to_chase[sub_entity_name::CHARACTER_CROSSHAIR];

	/* if we set player and crosshair entity targets */
	/* skip calculations if no orbit_mode is specified */
	if (crosshair_entity.alive()) {
		auto& crosshair = crosshair_entity.get<components::crosshair>();
		if (crosshair.orbit_mode != components::crosshair::NONE) {
			camera_crosshair_offset = components::crosshair::calculate_aiming_displacement(crosshair_entity, false);

			if (crosshair.orbit_mode == crosshair.ANGLED)
				camera_crosshair_offset.set_length(angled_look_length);

			if (crosshair.orbit_mode == crosshair.LOOK) {
				/* simple proportion */
				camera_crosshair_offset /= crosshair.bounds_for_base_offset;
				camera_crosshair_offset *= crosshair.max_look_expand;
			}
		}
	}

	return camera_crosshair_offset;
}