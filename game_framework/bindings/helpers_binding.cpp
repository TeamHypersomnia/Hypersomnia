#pragma once
#include "stdafx.h"
#include "bindings.h"

#include "../game/polygon_fader.h"

namespace bindings {
	luabind::scope _polygon_fader() {
		return
			luabind::class_<polygon_fader>("polygon_fader")
			.def(luabind::constructor<>())
			.def("add_trace", &polygon_fader::add_trace)
			.def("loop", &polygon_fader::loop)
			.def("generate_triangles", &polygon_fader::generate_triangles)
			.def("get_num_traces", &polygon_fader::get_num_traces)
			;
	}
}