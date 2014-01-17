#pragma once
#include "stdafx.h"
#include <luabind/iterator_policy.hpp>
#include "bindings.h"

#include "../resources/render_info.h"

namespace bindings {
	luabind::scope _polygon() {
		return 	
			(
			luabind::class_<basic_polygon>("basic_polygon")
			.def(luabind::constructor<>())
			.def("add_vertex", (void (basic_polygon::*)(const vec2<>&))&basic_polygon::push_back)
			,
			
			luabind::class_<vertex>("vertex")
			.def(luabind::constructor<vec2<>>())
			.def(luabind::constructor<vec2<>, vec2<>, graphics::pixel_32, texture_baker::texture*>())
			.def("set_texcoord", &vertex::set_texcoord)
			.def_readwrite("pos", &vertex::pos)
			.def_readwrite("texcoord", &vertex::texcoord)
			.def_readwrite("color", &vertex::color)
			,

			luabind::class_<vertex_triangle>("vertex_triangle")
			.def(luabind::constructor<>())
			.def("get_vert", &vertex_triangle::get_vert)
			,

			luabind::class_<polygon::concave>("drawable_concave")
			.def(luabind::constructor<>())
			.def("add_vertex", &polygon::concave::add_vertex)
			,

			luabind::class_<polygon, renderable>("polygon")
			.def(luabind::constructor<>())
			.def("add_concave", &polygon::add_concave)
			.def("get_vertex_count", &polygon::get_vertex_count)
			.def("get_vertex", &polygon::get_vertex)
			);
	}
}