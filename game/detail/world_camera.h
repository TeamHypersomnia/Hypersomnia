#pragma once
#include "augs/math/vec2.h"
#include "game/components/transform_component.h"
#include "augs/misc/smooth_value_field.h"
#include "game/detail/state_for_drawing_camera.h"

namespace augs {
	class variable_delta;
}

class interpolation_system;

class world_camera {
public:
	camera_cone camera;
	camera_cone smoothed_camera;

	float angled_look_length = 100.f;
	bool enable_smoothing = true;
	bool dont_smooth_once = false;

	float smoothing_average_factor = 0.5;
	float averages_per_sec = 25;

	components::transform last_interpolant;
	vec2 last_ortho_interpolant;

	vec2 previous_seen_player_position;
	vec2 previous_step_player_position;

	augs::smooth_value_field smoothing_player_pos;

	void configure_size(vec2);

	void tick(const interpolation_system& interp, augs::variable_delta dt, const_entity_handle entity_to_chase);
	state_for_drawing_camera get_state_for_drawing_camera(const_entity_handle entity_to_chase);

	vec2i get_camera_offset_due_to_character_crosshair(const_entity_handle) const;
};