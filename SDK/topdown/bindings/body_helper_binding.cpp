#pragma once
#include "stdafx.h"
#include "bindings.h"

#include "../game/body_helper.h"

namespace bindings {
	luabind::scope _body_helper() {
		return (
			luabind::class_ <physics_info>("physics_info")
			.def(luabind::constructor<>())
			.def_readwrite("rect_size", &physics_info::rect_size)
			.def_readwrite("type", &physics_info::type)
			.def_readwrite("filter", &physics_info::filter)
			.def_readwrite("density", &physics_info::density)
			.def_readwrite("angular_damping", &physics_info::angular_damping)
			.def_readwrite("linear_damping", &physics_info::linear_damping)
			.def_readwrite("set_fixed_rotation", &physics_info::fixed_rotation)
			.def("add_vertex", &physics_info::add_vertex)
			.enum_("constants")[
				luabind::value("POLYGON", physics_info::POLYGON),
				luabind::value("RECT", physics_info::RECT)
			],

			luabind::def("create_physics_component", create_physics_component)
		);
	}
}