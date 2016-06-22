#pragma once
#include "math/vec2.h"
#include "render_component.h"
#include "misc/fixed_delta_timer.h"

#include "game/entity_id.h"

#include "game/components/transform_component.h"
#include "game/detail/state_for_drawing.h"

#include "augs/misc/smooth_value_field.h"

class camera_system;
class gun_system;
class cosmos;

namespace components {
	struct camera  {
		enum orbit_type {
			NONE,
			ANGLED,
			LOOK
		} orbit_mode = NONE;

		static void configure_camera_and_character_with_crosshair(entity_handle camera, entity_handle character, entity_handle crosshair);

		augs::rects::xywh<int> viewport;
		vec2 visible_world_area;

		unsigned layer = 0;
		unsigned mask = 0;
		bool enabled = true;

		float angled_look_length = 100.f;
		bool enable_smoothing = true;
		bool dont_smooth_once = false;

		float smoothing_average_factor = 0.004f;
		float averages_per_sec = 60.0f;

		components::transform last_interpolant;

		vec2 rendered_size;

		vec2 max_look_expand = vec2(600.f, 300.f);

		entity_id entity_to_chase;

		vec2 previous_seen_player_position;
		vec2 previous_step_player_position;

		augs::smooth_value_field smoothing_player_pos;
		
		vec2i get_camera_offset_due_to_character_crosshair(cosmos& cosmos) const;
		
		shared::state_for_drawing_camera how_camera_will_render;

	private:
		friend class camera_system;
		friend class gun_system;

		vec2 last_ortho_interpolant;
	};
}
