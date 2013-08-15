#pragma once
#include "entity_system/component.h"
#include "math/vec2d.h"
#include "render_component.h"
#include "utility/delta_accumulator.h"

namespace augmentations {
	namespace entity_system {
		class entity;
	}
}

namespace components {
	struct camera : public augmentations::entity_system::component {
		augmentations::rects::xywh screen_rect;
		augmentations::rects::ltrb ortho;
		unsigned layer;
		unsigned mask;
		bool enabled;

		enum {
			NONE,
			ANGLED,
			LOOK
		} orbit_mode;
		float angled_look_length;
		bool enable_smoothing;
		double smoothing_average_factor, averages_per_sec;

		augmentations::vec2<> max_look_expand, last_interpolant;

		augmentations::entity_system::entity* player;
		augmentations::entity_system::entity* crosshair;

		augmentations::util::timer smooth_timer;

		camera(augmentations::rects::xywh screen_rect, augmentations::rects::ltrb ortho, unsigned layer, unsigned mask, 
			double smoothing_average_factor = 0.004, double averages_per_sec = 60.0) :
			screen_rect(screen_rect), ortho(ortho), layer(layer), mask(mask), enabled(true), orbit_mode(NONE), player(nullptr), crosshair(nullptr),
			angled_look_length(100.f), max_look_expand(augmentations::vec2<double>(600.f, 300.f)), 
			smoothing_average_factor(smoothing_average_factor), averages_per_sec(averages_per_sec), enable_smoothing(true) {
				smooth_timer.reset();
		}
	};
}
