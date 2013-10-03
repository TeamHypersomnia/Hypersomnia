#pragma once
#include "stdafx.h"
#include "bindings.h"


#include "../components/gun_component.h"

namespace bindings {
	luabind::scope _gun_component() {
		return
			luabind::class_<gun>("gun_component")
			.def(luabind::constructor<>())
			.def_readwrite("current_rounds", &gun::current_rounds)
			.def_readwrite("reloading", &gun::reloading)
			.def_readwrite("trigger", &gun::trigger)
			.def_readwrite("target_camera_to_shake", &gun::target_camera_to_shake)
			.def_readwrite("bullets_once", &gun::bullets_once)
			.def_readwrite("max_rounds", &gun::max_rounds)
			.def_readwrite("spread_degrees", &gun::spread_degrees)
			.def_readwrite("bullet_min_damage", &gun::bullet_min_damage)
			.def_readwrite("bullet_max_damage", &gun::bullet_max_damage)
			.def_readwrite("bullet_speed", &gun::bullet_speed)
			.def_readwrite("shooting_interval_ms", &gun::shooting_interval_ms)
			.def_readwrite("velocity_variation", &gun::velocity_variation)
			.def_readwrite("max_bullet_distance", &gun::max_bullet_distance)
			.def_readwrite("bullet_distance_offset", &gun::bullet_distance_offset)
			.def_readwrite("shake_radius", &gun::shake_radius)
			.def_readwrite("shake_spread_degrees", &gun::shake_spread_degrees)
			.def_readwrite("is_automatic", &gun::is_automatic)
			.def_readwrite("bullet_render", &gun::bullet_render)
			.def_readwrite("bullet_collision_filter", &gun::bullet_collision_filter);
	}
}