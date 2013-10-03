#include "stdafx.h"
#include "animate_info.h"

namespace resources {
	animation::frame::frame(sprite model, float duration_milliseconds)
		: model(model), duration_milliseconds(duration_milliseconds) {}

	animation::animation() : loop_mode(loop_type::REPEAT) {}
}

