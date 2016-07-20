#include "stdafx.h"
#include "bindings.h"

#include "game/bindings/bind_game_and_augs.h"
#include "augs/scripting/lua_state_wrapper.h"

namespace bindings {
	extern luabind::scope
		_vec2(),
		_rgba(),
		_rect_ltrb(),
		_rect_xywh(),
		_glwindow();
}

void bind_game_and_augs(augs::lua_state_wrapper& wrapper) {
	auto& raw = wrapper.raw;
	luabind::open(raw);

	luabind::module(raw)[
			bindings::_rgba(),
			bindings::_rect_ltrb(),
			bindings::_rect_xywh(),
			bindings::_glwindow(),

			luabind::def("open_editor", lua_state_wrapper::open_editor),

			bindings::_vec2()
	];

	//wrapper.global("THIS_LUA_STATE", wrapper);
	//luabind::bind_class_info(raw);
}