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
#include "view/audiovisual_state/flashbang_math.h"
#include "application/input/input_settings.h"

extern float max_zoom_out_at_edges_v;

camera_eye world_camera::get_current_eye(const bool with_edge_zoomout) const
{
	auto output_eye = current_eye;
	//output_eye.transform.pos.x = (int(output_eye.transform.pos.x) / 3) * 3; 
	//output_eye.transform.pos.y = (int(output_eye.transform.pos.y) / 3) * 3; 

	if (with_edge_zoomout) {
		const auto max_zoom_out = max_zoom_out_at_edges_v;

		output_eye.zoom *= augs::interp(1.0f, max_zoom_out, current_edge_zoomout_mult);
	}

	return output_eye;
}

namespace augs {
	template <class T, class A = float>
	A calc_alpha(const T a, const T b, const T v) {
		// Check if a and b are the same to prevent division by zero
		if (a == b) {
			// Return 0 or another appropriate default value, 
			// or perhaps handle this case differently depending on your requirements
			return static_cast<A>(0);
		}
		return static_cast<A>(v - a) / static_cast<A>(b - a);
	}
}

void world_camera::tick(
	const vec2i screen_size,
	const vec2 nonzoomedout_visible_world_area,
	const interpolation_system& interp,
	const augs::delta dt,
	world_camera_settings settings,
	const const_entity_handle entity_to_chase,
	const vec2 mid_step_crosshair_displacement,
	const input_settings& input_cfg
) {
	if (/* minimized */ screen_size.is_zero()) {
		return;
	}

	const auto min_height_for_zoom = 500.0f;
	//const auto max_height_for_zoom = 1500.0f;

	const auto height = [&]() {
		if (entity_to_chase.alive()) {
			if (auto movement = entity_to_chase.find<components::movement>()) {
				return std::max(0.0f, movement->portal_inertia_ms);
			}
		}

		return 0.0f;
	}();

	const auto& cosm = entity_to_chase.get_cosmos();

	const auto enable_smoothing = settings.enable_smoothing && entity_to_chase.alive();
	const auto average_factor = settings.smoothing.average_factor;
	const auto averages_per_sec = settings.smoothing.averages_per_sec;
	
	const auto target_cone = [&]() {
		/* we obtain transform as a copy because we'll be now offsetting it by crosshair position */
		if (entity_to_chase.alive()) {
			auto cone = camera_eye();

			cone.transform = entity_to_chase.get_viewing_transform(interp);
			cone.transform.rotation = 0;
			cone.zoom = current_eye.zoom;

			return cone;
		}

		return current_eye;
	}();

	const vec2 camera_crosshair_offset = calc_camera_offset_due_to_character_crosshair(
		entity_to_chase,
		settings,
		screen_size,
		mid_step_crosshair_displacement,
		current_eye.zoom
	); 

#if 0
	const auto max_zoom_out = max_zoom_out_at_edges_v;

	camera_crosshair_offset /= augs::interp(1.0f, max_zoom_out, current_edge_zoomout_mult);
#endif

	current_eye = target_cone;
	current_eye.transform.pos += camera_crosshair_offset;

	if (enable_smoothing) {
		/* variable time step target_cone smoothing by averaging last position with the current */
		float averaging_constant = 1.0f - static_cast<float>(std::pow(average_factor, averages_per_sec * dt.in_seconds()));

		if (dont_smooth_once) {
			averaging_constant = 0.0f;
		}

		//if ((transform.pos - last_interpolant).length() < 2.0) last_interpolant = transform.pos;
		//else

		const vec2 target_pos = target_cone.transform.pos + camera_crosshair_offset;
		const auto target_rotation = target_cone.transform.rotation;

		const vec2 smoothed_part = camera_crosshair_offset;

		last_interpolant.pos = augs::interp(vec2d(last_interpolant.pos), vec2d(smoothed_part), averaging_constant);
		last_interpolant.rotation = augs::interp(last_interpolant.rotation, target_rotation, averaging_constant);

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

		if (dont_smooth_once) {
			current_eye.transform.pos = target_pos;
			current_eye.transform.rotation = target_rotation;

			last_interpolant.pos = camera_crosshair_offset;
		}
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

			if (dont_smooth_once) {
				player_position_previously_seen = player_position_at_previous_step = player_pos;
			}

			const auto dt_ms = cosm.get_fixed_delta().in_milliseconds();

			const auto pos_dt = player_pos - player_position_previously_seen;
			target_value = pos_dt * dt_ms;

			if (target_value.length() < vec2(additional_position_smoothing.value).length()) {
				// braking
				settings.additional_position_smoothing.averages_per_sec += 3.5;
			}
			else {
				settings.additional_position_smoothing.averages_per_sec += 1.5;
			}

			auto lookout_bound = 50;

			const auto surfing_zoom_out = std::max(1.0f, settings.surfing_zoom_out);
			const auto surfing_zoom = 1.0f / surfing_zoom_out;

			if (height > min_height_for_zoom) {
				lookout_bound = augs::interp(50, 400, augs::calc_alpha(1.0f, 2.5f, surfing_zoom_out));
				target_zoom = surfing_zoom;
			}
			else {
				if (target_zoom < 1.0f) {
					if (height <= 200.0f) {
						target_zoom = augs::interp(1.0f, surfing_zoom, height/200.0f);
					}
				}
			}

			if (target_value.length() > lookout_bound) {
				target_value.set_length(lookout_bound);
			}

			// LOG("%x, %x, %x", *(vec2*)&player_pos, *(vec2*)&player_position_at_previous_step, *(vec2*)&target_value);
		}
		else {
			target_zoom = 1.0f;
		}
		//else {
		//	target_value = chased_transform.interpolation_direction(previous);
		//	target_value.set_length(100);
		//	additional_position_smoothing.averages_per_sec = 5;
		//}

		additional_position_smoothing.target_value = target_value * (-1);

		auto smoothing_settings = settings.additional_position_smoothing;

		const bool surfing = std::abs(current_eye.zoom - 1.0f) > 0.01f;

		if (surfing) {
			additional_position_smoothing.target_value *= -1;
			smoothing_settings.averages_per_sec *= 1.f;
		}

		additional_position_smoothing.tick(dt, smoothing_settings);

		if (dont_smooth_once) {
			additional_position_smoothing.snap_value_to_target();
		}
	}
	else {
		target_zoom = 1.0f;
	}

	if (enable_smoothing) {
		current_eye.transform.pos = vec2(current_eye.transform.pos + additional_position_smoothing.value);
	}
	else {
		current_eye.transform.pos = vec2(current_eye.transform.pos);
	}

	dont_smooth_once = false;

	auto interp_zoom = [dt](auto& current, const auto& target, float in_avgs = 5.f, float out_avgs = 10.f) {
		if (std::abs(current - target) < 0.001f) {
			current = target;
			return;
		}

		if (target > current) {
			auto avgs_per_sec = in_avgs;
			float zoom_averaging_constant = 1.0f - static_cast<float>(std::pow(0.5f, avgs_per_sec * dt.in_seconds()));
			current = augs::interp(current, target, zoom_averaging_constant);
		}
		else {
			auto avgs_per_sec = out_avgs;
			float zoom_averaging_constant = 1.0f - static_cast<float>(std::pow(0.5f, avgs_per_sec * dt.in_seconds()));
			current = augs::interp(current, target, zoom_averaging_constant);
		}
	};

	target_edge_zoomout_mult = calc_camera_zoom_out_due_to_character_crosshair(
		entity_to_chase,
		settings,
		nonzoomedout_visible_world_area,
		mid_step_crosshair_displacement,
		current_edge_zoomout_mult,
		input_cfg
	);

	interp_zoom(current_eye.zoom, target_zoom);

	/* Here it's reversed */
	const float in_avgs = 12.0f;
	const float out_avgs = 5.0f;

	interp_zoom(current_edge_zoomout_mult, target_edge_zoomout_mult, out_avgs, in_avgs);

	if (target_edge_zoomout_mult == 0.0f && augs::is_epsilon(current_edge_zoomout_mult, 0.001f)) {
		current_edge_zoomout_mult = 0.0f;
	}

	advance_flash(entity_to_chase, dt);
}

float world_camera::calc_camera_zoom_out_due_to_character_crosshair(
	const const_entity_handle entity_to_chase,
	const world_camera_settings& settings,
	const vec2 nonzoomedout_visible_world_area,
	const vec2 mid_step_crosshair_displacement,
	const float current_edge_zoomout_mult,
	const input_settings& input_cfg
) {
	if (entity_to_chase.dead()) {
		return 0.0f;
	}

	if (const bool manual_zoom = !input_cfg.auto_zoom_out_near_screen_edges) {
		if (const auto crosshair = entity_to_chase.find_crosshair()) {
			if (crosshair->zoom_out_mode) {
				return 1.0f;
			}
		}

		return 0.0f;
	}

	const auto screen_size = nonzoomedout_visible_world_area;
	const auto zone = screen_size.bigger_side() * settings.edge_zoom_out_zone;

	if (zone == 0.0f) {
		return 0.0f;
	}

	auto crosshair = ::calc_crosshair_displacement(entity_to_chase);

	if (current_edge_zoomout_mult > settings.edge_zoom_in_cutoff_mult) {
		crosshair *= settings.edge_zoom_in_zone_expansion;
	}

	const auto dist_l = crosshair.x - (-screen_size.x);
	const auto dist_t = crosshair.y - (-screen_size.y);
	const auto dist_r = screen_size.x - crosshair.x;
	const auto dist_b = screen_size.y - crosshair.y;
	(void)mid_step_crosshair_displacement;

	const auto min_dist = std::max(0.0f, std::min({ dist_l, dist_t, dist_r, dist_b }));

	if (min_dist < zone) {
		return 1.0f - min_dist / zone;
	}

	return 0.0f;
}

vec2 world_camera::calc_camera_offset_due_to_character_crosshair(
	const const_entity_handle entity_to_chase,
	const world_camera_settings settings,
	const vec2 screen_size,
	const vec2 mid_step_crosshair_displacement,
	const float zoom
) {
	vec2 camera_crosshair_offset;

	if (entity_to_chase.dead()) {
		return { 0, 0 };
	}

	if (const auto crosshair = entity_to_chase.find_crosshair()) {
		if (crosshair->orbit_mode != crosshair_orbit_type::NONE) {
			camera_crosshair_offset = calc_crosshair_displacement(entity_to_chase) + mid_step_crosshair_displacement;

			if (crosshair->orbit_mode == crosshair_orbit_type::ANGLED) {
				camera_crosshair_offset.set_length(settings.angled_look_length);
			}

			if (crosshair->orbit_mode == crosshair_orbit_type::LOOK) {
				const auto bound = screen_size;

				if (bound.neither_zero()) {
					camera_crosshair_offset /= bound;
				}

				camera_crosshair_offset *= camera_cone({ vec2::zero, zoom }, screen_size).get_visible_world_area() * std::min(0.5f, settings.look_bound_expand) * zoom;
			}
		}
	}

	return camera_crosshair_offset;
}

void world_camera::advance_flash(const const_entity_handle viewer, const augs::delta dt) {
	request_afterimage = false;

	if (viewer.dead()) {
		return;
	}

	const auto sentience = viewer.template find<invariants::sentience>();

	if (sentience == nullptr) {
		return;
	}

	const auto mult = get_flash_visual_mult(viewer);

	auto& last_mult = last_registered_flash_mult;

	if (mult > 0.f) {
		if (mult > last_mult) {
			after_flash_passed_ms += dt.in_milliseconds();

			const auto delay_ms = sentience->flash_effect_delay_ms;

			if (after_flash_passed_ms >= delay_ms) {
				after_flash_passed_ms = 0;
				last_mult = mult;

				request_afterimage = true;
			}
		}
	}
	else {
		after_flash_passed_ms = 0.f;
	}

	if (mult <= last_mult) {
		last_mult = mult;
	}
}