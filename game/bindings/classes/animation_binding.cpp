#pragma once
#include "stdafx.h"
#include "bindings.h"
#include "bind_callbacks.h"

#include "../resources/animate_info.h"
#include "../shared/drawing_state.h"

namespace bindings {
	luabind::scope _animation() {
		return
			luabind::class_<animation::frame>("animation_frame")
			.def(luabind::constructor<>())
			.def_readwrite("duration_milliseconds", &animation::frame::duration_milliseconds)
			.def_readwrite("model", &animation::frame::sprite)
			.property("callback", bind_callback(&animation::frame::callback))
			.property("callback_out", bind_callback(&animation::frame::callback_out))
			,

			bind_stdvector<animation::frame>("animation_frame_vector"),

			luabind::class_<animation>("animation")
			.def(luabind::constructor<>())
			.def_readwrite("frames", &animation::frames)
			.def_readwrite("loop_mode", &animation::loop_mode)
			.enum_("loop_type")[
				luabind::value("REPEAT", animation::loop_type::REPEAT),
				luabind::value("INVERSE", animation::loop_type::INVERSE),
				luabind::value("NONE", animation::loop_type::NONE)
			];
	}
}