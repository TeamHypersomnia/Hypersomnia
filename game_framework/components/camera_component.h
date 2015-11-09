#pragma once
#include "entity_system/component.h"
#include "math/vec2.h"
#include "render_component.h"
#include "misc/delta_accumulator.h"

#include "entity_system/entity.h"

#include "../components/transform_component.h"
#include "../resources/render_info.h"

class camera_system;
class gun_system;

namespace components {
	struct camera : public augs::component {
		augs::rects::xywh<int> screen_rect;
		vec2 size;

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
		components::transform previously_drawn_at;

		vec2 rendered_size;

		vec2 max_look_expand = vec2(600.f, 300.f);

		augs::entity_id player, crosshair;

		camera() { smooth_timer.reset(); }

		/* arguments: subject, renderer, mask */
		std::function<void(augs::entity_id, resources::renderable::draw_input, int)> drawing_callback;

	private:
		friend class camera_system;
		friend class gun_system;

		augs::timer smooth_timer;
		vec2 last_ortho_interpolant;
	};
}
