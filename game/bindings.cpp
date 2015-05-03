#pragma once
#include "stdafx.h"
#include "utilities/lua_state_wrapper.h"
#include "bindings.h"
#include "utilities.h"

void bind_hypersomnia_implementations(augs::lua_state_wrapper& wrapper) {
	luabind::module(wrapper.raw)[
		luabind::def("draw_tile_highlights", draw_tile_highlights),
		luabind::def("get_random_coordinate_on_a_special_tile", get_random_coordinate_on_a_special_tile)
	];
}