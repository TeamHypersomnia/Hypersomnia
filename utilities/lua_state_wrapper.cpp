#pragma once
#include "stdafx.h"
#include "lua_state_wrapper.h"
#include "script.h"

namespace augs {
	lua_state_wrapper::lua_state_wrapper(const lua_state_wrapper&) {
		assert(0);
	}

	lua_state_wrapper::lua_state_wrapper() : raw(luaL_newstate()) {
		luaopen_base(raw);
		luaL_openlibs(raw);
	}

	lua_state_wrapper::~lua_state_wrapper() { lua_close(raw); }

	lua_state_wrapper::operator lua_State*() {
		return raw;
	}

	void lua_state_wrapper::dofile(const std::string& filename) {
		script my_script(*this);
		my_script.associate_filename(filename);
		my_script.call();
	}

	std::string lua_state_wrapper::get_traceback() {
		lua_State* L = raw;

		lua_getglobal(L, "debug");
		lua_getfield(L, -1, "traceback");
		lua_pushvalue(L, 1);
		lua_pushinteger(L, 2);
		lua_call(L, 2, 1);

		return std::string(lua_tostring(L, -1));
	}
}