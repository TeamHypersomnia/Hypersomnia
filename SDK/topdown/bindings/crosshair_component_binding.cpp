#pragma once
#include "stdafx.h"
#include "bindings.h"


#include "../components/crosshair_component.h"

namespace bindings {
	luabind::scope _crosshair_component() {
		return			
			luabind::class_<crosshair>("crosshair_component")
			.def(luabind::constructor<>())
			//.def_readwrite("bounds", &components::crosshair::bounds)
			//.def_readwrite("blink", &components::crosshair::blink)
			.def_readwrite("should_blink", &crosshair::should_blink)
			.def_readwrite("sensitivity", &crosshair::sensitivity);
	}
}