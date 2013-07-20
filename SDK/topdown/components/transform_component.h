#pragma once
#include "../../../entity_system/entity_system.h"
#include "../../../rects/rects.h"

namespace components {
	struct transform : public augmentations::entity_system::component {
		augmentations::rects::pointf pos;
		augmentations::rects::wh size;
		double rotation;
		transform() : rotation(0.0) { }
	};
}