#pragma once
#include "stdafx.h"
#include "bindings.h"

#include "window_framework/window.h"

namespace bindings {
	luabind::scope _glwindow() {
		return
			luabind::class_<window::glwindow>("glwindow")
			.def(luabind::constructor<>());
	}
}