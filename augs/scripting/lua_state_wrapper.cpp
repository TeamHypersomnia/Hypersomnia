#pragma once
#include "lua/lua.hpp"
#include <luabind/luabind.hpp>
#include "lua_state_wrapper.h"
#include "script.h"
#include "augs/window_framework/platform_utils.h"

#include "log.h"

// called by luabind internals whenever an error occurs
void luabind_error_callback(lua_State *L) {
	augs::lua_state_wrapper(L).debug_response();
}

namespace augs {
	lua_state_wrapper::lua_state_wrapper(lua_State* state) : raw(state) {
		owns = false;
	}
	
	lua_state_wrapper::lua_state_wrapper() : raw(luaL_newstate()) {
		luaopen_base(raw);
		luaL_openlibs(raw);
	}

	lua_state_wrapper::~lua_state_wrapper() { 
		if(owns) 
			lua_close(raw);
	}

	lua_state_wrapper::operator lua_State*() {
		return raw;
	}

	bool lua_state_wrapper::dofile(const std::string& filename) {
		script my_script(*this);
		my_script.associate_filename(filename);
		return my_script.call();
	}


	std::string lua_state_wrapper::get_error_and_stack() {
		lua_State* L = raw;
		std::string error_message = "\n";
		const char* str = lua_tostring(L, -1);
		if (str) error_message += std::string(str);
		lua_pop(L, 1);

		lua_getglobal(L, "debug");
		lua_getfield(L, -1, "traceback");
		lua_pushvalue(L, 1);
		lua_pushinteger(L, 2);
		lua_call(L, 2, 1);

		const char* stack = lua_tostring(L, -1);
		error_message += stack ? stack : "\nFailed to retrieve the lua call stack!";

		return error_message;
	}

	void lua_state_wrapper::call_traceback_that_saves_verbose_log() {
		lua_State* L = raw;
		lua_getglobal(L, "debug");
		lua_getfield(L, -1, "post_traceback");
		lua_pushvalue(L, 1);
		lua_pushinteger(L, 2);
		lua_pcall(L, 2, 1, 0);
	}

	std::string lua_state_wrapper::open_editor(std::string error_message) {
		if (error_message.empty()) 
			return "";

		std::stringstream ss(error_message);
		std::string to;

		std::string full_command = std::string("\"") + 
		getenv("AUG_SCRIPTEDITOR") + 
		//"C:\\Program Files (x86)\\Notepad++\\notepad++.exe"
		"\" ";
		std::string lines;

		int lines_found = 0;

		while (std::getline(ss, to, '\n')) {
			std::stringstream line(to);
			std::string to2;
			line >> to2;
			if (to2.empty()) continue;

			to2.erase(to2.end() - 1);
			if (to2.find(".lua") != std::string::npos) {
				auto ws = window::get_executable_path();
				std::string exe_path(ws.begin(), ws.end());
				std::string full_file_path = (exe_path + "\\" + to2);
				
				lines += full_file_path + "\n";
				full_command += full_file_path + " ";

				++lines_found;
			}
		}

		if(lines_found > 0)
			CALL_SHELL(full_command);

		return lines;
	}

	void lua_state_wrapper::debug_response() {
		std::string error_and_stack = get_error_and_stack();
		std::string lines = open_editor(error_and_stack);
		call_traceback_that_saves_verbose_log();

		LOG("%x\n%x", error_and_stack, lines);
	}
}