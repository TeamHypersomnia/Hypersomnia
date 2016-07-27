#pragma once
#include "augs/math/vec2.h"
#include "render_component.h"
#include "augs/misc/fixed_delta_timer.h"

#include "game/transcendental/entity_id.h"

#include "game/components/transform_component.h"
#include "game/detail/state_for_drawing_camera.h"

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

		vec2 visible_world_area;

		float angled_look_length = 100.f;
		bool enable_smoothing = true;
		bool dont_smooth_once = false;

		float smoothing_average_factor = 0.004f;
		float averages_per_sec = 60.0f;

		components::transform last_interpolant;

		vec2 max_look_expand = vec2(600.f, 300.f);

		entity_id entity_to_chase;

		vec2 previous_seen_player_position;
		vec2 previous_step_player_position;

		augs::smooth_value_field smoothing_player_pos;
		
		state_for_drawing_camera how_camera_will_render;

		template <class Archive>
		void serialize(Archive& ar) {
			ar(
				CEREAL_NVP(orbit_mode),

				CEREAL_NVP(visible_world_area),

				CEREAL_NVP(angled_look_length),
				CEREAL_NVP(enable_smoothing),
				CEREAL_NVP(dont_smooth_once),

				CEREAL_NVP(smoothing_average_factor),
				CEREAL_NVP(averages_per_sec),

				CEREAL_NVP(last_interpolant),

				CEREAL_NVP(max_look_expand),

				CEREAL_NVP(entity_to_chase),

				CEREAL_NVP(previous_seen_player_position),
				CEREAL_NVP(previous_step_player_position),

				CEREAL_NVP(smoothing_player_pos),

				CEREAL_NVP(how_camera_will_render)
			);
		}

		static void configure_camera_and_character_with_crosshair(entity_handle camera, entity_handle character, entity_handle crosshair);
		vec2i get_camera_offset_due_to_character_crosshair(const cosmos& cosmos) const;

	private:
		friend class camera_system;
		friend class gun_system;

		vec2 last_ortho_interpolant;
	};
}
