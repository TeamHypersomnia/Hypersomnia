#pragma once
#include "stdafx.h"
#include "bindings.h"

#include "../resources/render_info.h"

namespace bindings {
	luabind::scope _polygon() {
		return 	
			(
			luabind::class_<vertex>("vertex")
			.def(luabind::constructor<vec2<>, vec2<>, graphics::pixel_32, texture_baker::texture*>())
			,

			luabind::class_<polygon::concave>("drawable_concave")
			.def(luabind::constructor<>())
			.def("add_vertex", &polygon::concave::add_vertex)
			,

			luabind::class_<polygon, renderable>("polygon")
			.def(luabind::constructor<>())
			.def("add_concave", &polygon::add_concave)
			);
	}
}