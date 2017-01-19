#include "stdafx.h"
#include "bindings.h"

#include "game/bindings/bind_game_and_augs.h"
#include "augs/scripting/lua_state_raii.h"

#include "augs/log.h"
#include <luabind/class_info.hpp>

namespace bindings {
	extern luabind::scope
		_vec2(),
		_rgba(),
		_rect_ltrb(),
		_rect_xywh(),
		_glwindow();
}

static void LOG_surrogate(const std::string& s) {
	LOG(s);
}

void bind_game_and_augs(augs::lua_state_raii& wrapper) {
	lua_State* raw = wrapper;

	luabind::open(raw);

	luabind::module(raw)[
			bindings::_rgba(),
			bindings::_rect_ltrb(),
			bindings::_rect_xywh(),
			bindings::_glwindow(),

			luabind::def("LOG", LOG_surrogate),

			bindings::_vec2()
	];

	//wrapper.global("THIS_LUA_STATE", wrapper);
	luabind::bind_class_info(raw);
}