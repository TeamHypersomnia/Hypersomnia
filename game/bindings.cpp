#pragma once
#include "stdafx.h"
#include "utilities/lua_state_wrapper.h"

#include "../gui/hypersomnia_gui.h"

#include "hypersomnia_world.h"

#include "bindings.h"
#include "utilities.h"

void hypersomnia_world::bind_this_to_lua_global(lua_state_wrapper& lua, std::string global) {
	lua.global_ptr(global, this);
}

void bind_whole_hypersomnia(augs::lua_state_wrapper& wrapper) {
	hypersomnia_gui::bind(wrapper);

	luabind::module(wrapper.raw)[
		luabind::class_<hypersomnia_world, augs::world>("hypersomnia_world")
		.def(luabind::constructor<>())
			,

		// bottlenecks
		luabind::def("draw_tile_highlights", draw_tile_highlights),
		luabind::def("get_random_coordinate_on_a_special_tile", get_random_coordinate_on_a_special_tile)
	];
}