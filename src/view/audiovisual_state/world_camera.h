#pragma once
#include "augs/math/vec2.h"
#include "game/components/transform_component.h"
#include "augs/misc/smooth_value_field.h"
#include "augs/math/camera_cone.h"
#include "game/cosmos/entity_handle_declaration.h"

namespace augs {
	class delta;
}

struct input_settings;

class interpolation_system;

struct world_camera_settings {
	// GEN INTROSPECTOR struct world_camera_settings
	augs::smoothing_settings<double> additional_position_smoothing;
	augs::smoothing_settings<float> smoothing;

	float angled_look_length = 100.f;
	float look_bound_expand = 0.5f;
	float surfing_zoom_out = 2.5f;
	bool enable_smoothing = true;

	float edge_zoom_out_zone = 0.005f;
	float edge_zoom_in_cutoff_mult = 0.03f;
	float edge_zoom_in_zone_expansion = 5.0f;
	// END GEN INTROSPECTOR

	bool operator==(const world_camera_settings& b) const = default;
};

struct world_camera {
	bool dont_smooth_once = false;

	transformr last_interpolant;

	vec2 player_position_previously_seen;
	vec2 player_position_at_previous_step;

	augs::smooth_value_field additional_position_smoothing;
	float target_zoom = 1.0f;

	float current_edge_zoomout_mult = 0.0f;

	camera_eye get_current_eye(bool with_edge_zoomout) const;

	void tick(
		const vec2i screen_size,
		const vec2 nonzoomedout_visible_world_area,
		const interpolation_system& interp, 
		augs::delta dt,
		world_camera_settings settings,
		const_entity_handle entity_to_chase,
		const vec2 mid_step_crosshair_displacement,
		const input_settings& input_cfg
	);

	auto get_effective_flash_mult() const {
		return last_registered_flash_mult;
	}

	bool is_flash_afterimage_requested() const {
		return request_afterimage;
	}

private:
	camera_eye current_eye;

	void advance_flash(const_entity_handle viewer, augs::delta dt);
	float target_edge_zoomout_mult = 1.0f;

	float after_flash_passed_ms = 0.f;
	float last_registered_flash_mult = 0.f;
	bool request_afterimage = false;

	static vec2 calc_camera_offset_due_to_character_crosshair(
		const const_entity_handle,
		const world_camera_settings,
		const vec2 screen_size,
		const vec2 crosshair_displacement,
		const float zoom
	);

	static float calc_camera_zoom_out_due_to_character_crosshair(
		const const_entity_handle,
		const world_camera_settings&,
		const vec2 nonzoomedout_visible_world_area,
		const vec2 crosshair_displacement,
		const float current_edge_zoomout_mult,
		const input_settings& input_cfg
	);
};