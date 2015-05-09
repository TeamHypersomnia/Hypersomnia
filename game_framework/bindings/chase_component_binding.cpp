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
			.def_readwrite("target", &chase::target)
			.def_readwrite("chase_type", &chase::chase_type)
			.def_readwrite("reference_position", &chase::reference_position)
			.def_readwrite("target_reference_position", &chase::target_reference_position)
			.def_readwrite("scrolling_speed", &chase::scrolling_speed)
			.def_readwrite("offset", &chase::offset)
			.def_readwrite("rotation_orbit_offset", &chase::rotation_orbit_offset)
			.def_readwrite("rotation_offset", &chase::rotation_offset)
			.def_readwrite("relative", &chase::relative)
			.def_readwrite("chase_rotation", &chase::chase_rotation)
			.def_readwrite("track_origin", &chase::track_origin)
			.def_readwrite("rotation_multiplier", &chase::rotation_multiplier)
			.def_readwrite("subscribe_to_previous", &chase::subscribe_to_previous)
			.enum_("chase_type")[
				luabind::value("OFFSET", chase::chase_type::OFFSET),
					luabind::value("PARALLAX", chase::chase_type::PARALLAX),
					luabind::value("ORBIT", chase::chase_type::ORBIT)
			];
	}
}