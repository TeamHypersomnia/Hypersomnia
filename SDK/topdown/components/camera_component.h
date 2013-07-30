#pragma once
#include "../../../entity_system/entity_system.h"
#include "../../../math/rects.h"
#include "render_component.h"

namespace components {
	struct camera : public augmentations::entity_system::component {
		augmentations::rects::xywh screen_rect;
		augmentations::rects::ltrb ortho;
		unsigned layer;
		unsigned mask;
		bool enabled;

		camera(augmentations::rects::xywh screen_rect, augmentations::rects::ltrb ortho, unsigned layer, unsigned mask) :
			screen_rect(screen_rect), ortho(ortho), layer(layer), mask(mask), enabled(true) {}
	};
}
