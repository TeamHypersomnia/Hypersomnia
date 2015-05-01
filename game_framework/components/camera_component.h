#pragma once
#include "entity_system/component.h"
#include "math/vec2d.h"
#include "render_component.h"
#include "misc/delta_accumulator.h"

#include "entity_system/entity_ptr.h"

#include "../components/transform_component.h"

class camera_system;
class gun_system;

namespace components {
	struct camera : public augs::entity_system::component {
		augs::rects::xywh<float> screen_rect;
		augs::vec2<> size;

		unsigned layer;
		unsigned mask;
		bool enabled;

		enum orbit_type {
			NONE,
			ANGLED,
			LOOK
		} orbit_mode;

		float angled_look_length;
		bool enable_smoothing;
		bool dont_smooth_once = false;
		bool crosshair_follows_interpolant;

		double smoothing_average_factor, averages_per_sec;
		components::transform::state<double> last_interpolant;

		augs::vec2<> rendered_size;

		augs::vec2<> max_look_expand;

		augs::entity_system::entity_ptr player, crosshair;

		camera(augs::rects::xywh<float> screen_rect = augs::rects::xywh<float>(), augs::vec2<> size = augs::vec2<>(),
			unsigned layer = 0, unsigned mask = 0,
			double smoothing_average_factor = 0.004, double averages_per_sec = 60.0) :
			screen_rect(screen_rect), size(size), layer(layer), mask(mask), enabled(true), orbit_mode(NONE), player(nullptr), crosshair(nullptr),
			angled_look_length(100.f), max_look_expand(augs::vec2<double>(600.f, 300.f)), 
			smoothing_average_factor(smoothing_average_factor), averages_per_sec(averages_per_sec), enable_smoothing(true), crosshair_follows_interpolant(false) {
				smooth_timer.reset();
		}

		luabind::object drawing_callback;

	private:
		friend class camera_system;
		friend class gun_system;

		augs::misc::timer smooth_timer;
		augs::vec2<> last_ortho_interpolant;
	};
}
