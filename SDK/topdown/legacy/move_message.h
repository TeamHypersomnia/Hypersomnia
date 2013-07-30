#pragma once
#include "../../../math/vec2d.h"
#include "message.h"

namespace messages {
	struct move_message : public message {
		augmentations::vec2<float> new_pos;
	};
}