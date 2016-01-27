#pragma once
#include "stdafx.h"
#include "utilities/lua_state_wrapper.h"

#include "game_framework/game_world.h"

#include "bindings.h"
#include "utilities.h"

void bind_whole_hypersomnia(augs::lua_state_wrapper& wrapper) {
	luabind::module(wrapper.raw)[
	];
}