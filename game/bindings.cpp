#pragma once
#include "stdafx.h"
#include "utilities/lua_state_wrapper.h"

#include "hypersomnia_world.h"

#include "bindings.h"
#include "utilities.h"

void game_world::bind_this_to_lua_global(lua_state_wrapper& lua, std::string global) {
	lua.global_ptr(global, this);
}

void bind_whole_hypersomnia(augs::lua_state_wrapper& wrapper) {
	luabind::module(wrapper.raw)[
		luabind::class_<game_world, augs::world>("hypersomnia_world")
		.def(luabind::constructor<overworld&>())
	];
}