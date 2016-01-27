#pragma once
#include "entity_system/component.h"
#include "math/vec2.h"
#include "render_component.h"
#include "misc/fixed_delta_timer.h"

#include "entity_system/entity.h"

#include "../components/transform_component.h"
#include "../shared/drawing_state.h"

#include "utilities/misc/smooth_value_field.h"

class camera_system;
class gun_system;

namespace components {
	struct camera : public augs::component {
		static void configure_camera_player_crosshair(augs::entity_id camera, augs::entity_id player, augs::entity_id crosshair);

		augs::rects::xywh<int> viewport;
		vec2 visible_world_area;

		unsigned layer = 0;
		unsigned mask = 0;
		bool enabled = true;

		enum orbit_type {
			NONE,
			ANGLED,
			LOOK
		} orbit_mode = NONE;

		float angled_look_length = 100.f;
		bool enable_smoothing = true;
		bool dont_smooth_once = false;
		bool crosshair_follows_interpolant = false;

		float smoothing_average_factor = 0.004f;
		float averages_per_sec = 60.0f;

		components::transform last_interpolant;

		vec2 rendered_size;

		vec2 max_look_expand = vec2(600.f, 300.f);

		augs::entity_id player, crosshair;
		vec2 previous_seen_player_position;
		vec2 previous_step_player_position;

		augs::smooth_value_field smoothing_player_pos;

		struct constraint_output {
			vec2i camera_crosshair_offset;
			vec2 constrained_crosshair_pos;
			vec2 constrained_crosshair_base_offset;

		} get_constrained_crosshair_and_camera_offset(augs::entity_id self);
		
		shared::drawing_state how_camera_will_render;

	private:
		friend class camera_system;
		friend class gun_system;

		augs::timer smooth_timer;
		vec2 last_ortho_interpolant;
	};
}
