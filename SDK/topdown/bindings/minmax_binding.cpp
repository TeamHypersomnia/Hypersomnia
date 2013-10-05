#pragma once
#include "stdafx.h"
#include "bindings.h"

namespace bindings {
	luabind::scope _minmax() {
		return (
			luabind::class_<std::pair<float, float>>("minmax")
				.def(luabind::constructor<float, float>())
				.def_readwrite("min", &std::pair<float, float>::first)
				.def_readwrite("max", &std::pair<float, float>::second),

			luabind::class_<std::pair<unsigned, unsigned>>("minmax_u")
				.def(luabind::constructor<unsigned, unsigned>())
				.def_readwrite("min", &std::pair<unsigned, unsigned>::first)
				.def_readwrite("max", &std::pair<unsigned, unsigned>::second)
			);
	}
}