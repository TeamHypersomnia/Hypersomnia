#pragma once
#include "stdafx.h"
#include "bindings.h"


#include "../components/health_component.h"

namespace bindings {
	luabind::scope _health_component() {
		return
			luabind::class_<health>("health_component")
			.def(luabind::constructor<>())
			.def_readwrite("hp", &health::hp)
			.def_readwrite("dead", &health::dead)
			.def_readwrite("death_render", &health::death_render)
			.def_readwrite("max_hp", &health::max_hp)
			.def_readwrite("should_disappear", &health::should_disappear)
			.def_readwrite("dead_lifetime_ms", &health::dead_lifetime_ms)
			.def_readwrite("corpse_collision_filter", &health::corpse_collision_filter);
	}
}