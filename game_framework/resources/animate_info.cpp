#include "stdafx.h"
#include "animate_info.h"

namespace resources {
	animation::frame::frame(sprite model, float duration_milliseconds, luabind::object callback, luabind::object callback_out)
		: model(model), duration_milliseconds(duration_milliseconds), callback(callback), callback_out(callback_out) {}

	animation::animation() : loop_mode(loop_type::REPEAT) {}
}

