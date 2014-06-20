#pragma once
#include "stdafx.h"
#include "lua_state_wrapper.h"
#include "script.h"

namespace augs {
	lua_state_wrapper::lua_state_wrapper(const lua_state_wrapper&) {
		assert(0);
	}

	lua_state_wrapper::lua_state_wrapper() : raw(luaL_newstate()) {}
	lua_state_wrapper::~lua_state_wrapper() { lua_close(raw); }

	lua_state_wrapper::operator lua_State*() {
		return raw;
	}

	void lua_state_wrapper::dofile(const std::string& filename) {
		script my_script(*this);
		my_script.associate_filename(filename);
		my_script.call();
	}
}