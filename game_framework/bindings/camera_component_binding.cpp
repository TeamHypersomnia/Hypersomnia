#pragma once
#include "stdafx.h"
#include "bindings.h"


#include "../components/camera_component.h"

namespace bindings {
	luabind::scope _camera_component() {
		return
			luabind::class_<camera>("camera_component")
			.def(luabind::constructor<>())
			.def_readwrite("screen_rect", &camera::screen_rect)
			.def_readwrite("size", &camera::size)
			.def_readwrite("layer", &camera::layer)
			.def_readwrite("mask", &camera::mask)
			.def_readwrite("enabled", &camera::enabled)
			.def_readwrite("orbit_mode", &camera::orbit_mode)
			.def_readwrite("angled_look_length", &camera::angled_look_length)
			.def_readwrite("enable_smoothing", &camera::enable_smoothing)
			.def_readwrite("smoothing_average_factor", &camera::smoothing_average_factor)
			.def_readwrite("averages_per_sec", &camera::averages_per_sec)
			.def_readwrite("last_interpolant", &camera::last_interpolant)
			.def_readwrite("max_look_expand", &camera::max_look_expand)
			.def_readwrite("player", &camera::player)
			.def_readwrite("crosshair", &camera::crosshair)
			.def_readwrite("drawing_callback", &camera::drawing_callback)
			.def_readwrite("crosshair_follows_interpolant", &camera::crosshair_follows_interpolant)
			.enum_("orbit_type")[
				luabind::value("NONE", camera::orbit_type::NONE),
				luabind::value("ANGLED", camera::orbit_type::ANGLED),
				luabind::value("LOOK", camera::orbit_type::LOOK)
			];
	}
}