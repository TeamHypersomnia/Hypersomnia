#pragma once
#include "stdafx.h"
#include "bindings.h"

#include "../components/chase_component.h"

namespace bindings {
	luabind::scope _chase_component() {
		return
			luabind::class_<chase>("chase_component")
			.def(luabind::constructor<>())
			.def("set_target", &chase::set_target)
			//.def_readwrite("target", &chase::target)
			.def_readwrite("chase_type", &chase::chase_type)
			.def_readwrite("offset", &chase::offset)
			.def_readwrite("rotation_orbit_offset", &chase::rotation_orbit_offset)
			.def_readwrite("rotation_offset", &chase::rotation_offset)
			.def_readwrite("relative", &chase::relative)
			.def_readwrite("chase_rotation", &chase::chase_rotation)
			.def_readwrite("track_origin", &chase::track_origin)
			.enum_("chase_type")[
				luabind::value("OFFSET", chase::chase_type::OFFSET),
					luabind::value("ORBIT", chase::chase_type::ORBIT)
			];
	}
}