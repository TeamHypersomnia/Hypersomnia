#include "stdafx.h"
#include "script.h"
#include "lua_state_wrapper.h"

#include <fstream>
#include <iostream>

namespace augs {
	script::script(lua_state_wrapper& lua_state) : lua_state(lua_state), needs_recompilation(false), is_associated_string_filename(false) {

	}

	void script::associate_string(const std::string& str) {
		associated_string = str;
		is_associated_string_filename = false;
		needs_recompilation = true;
	}
	
	void script::associate_filename(const std::string& str) {
		associated_string = str;
		is_associated_string_filename = true;
		needs_recompilation = true;
	}

	const std::string& script::get_associated_string() const {
		return associated_string;
	}

	bool script::is_filename() const {
		return is_associated_string_filename;
	}

	const std::vector<char>& script::get_bytecode() const {
		return bytecode;
	}

	int lua_writer(lua_State *L,
		const void* p,
		size_t sz,
		void* ud) {
			auto* ptr = static_cast<const char*>(p);
			auto& char_vec = *static_cast<std::vector<char>*>(ud);

			char_vec.resize(char_vec.size() + sz);
			memcpy(char_vec.data() + char_vec.size() - sz, ptr, sz);
			return 0;
	}

	const char* lua_reader(lua_State *L, void *data, size_t *sz) {
		auto& info = *static_cast<std::pair<bool, std::vector<char>*>*>(data);
		auto& bytecode = *info.second;

		if (info.first) {
			info.first = false;
			*sz = bytecode.size();
			return bytecode.data();
		}
		else return nullptr;
	}

	void script::set_out_of_date() {
		needs_recompilation = true;
	}

	std::string script::compile() {
		if (needs_recompilation) {
			int result = 0;
			bytecode.clear();

			if (is_associated_string_filename) {
				std::ifstream t(associated_string);
				std::string script_file;

				t.seekg(0, std::ios::end);
				script_file.reserve(static_cast<unsigned>(t.tellg()));
				t.seekg(0, std::ios::beg);

				script_file.assign((std::istreambuf_iterator<char>(t)),
					std::istreambuf_iterator<char>());

				result = luaL_loadstring(lua_state, script_file.c_str());
			}
			else
				result = luaL_loadstring(lua_state, associated_string.c_str());

			if (result != 0)
				return lua_tostring(lua_state, -1);
			else {
				lua_dump(lua_state, lua_writer, &bytecode);
				lua_pop(lua_state, 1);
			}
		}

		needs_recompilation = false;
		return std::string();
	}

	void script::report_errors(std::string& errors) {
		if (is_associated_string_filename) {
			std::cout << associated_string << ": ";
		}

		std::cout << errors << std::endl;
	}

	std::string script::call() {
		auto compilation_error = compile();
		if (!compilation_error.empty()) {
			report_errors(compilation_error);
			return compilation_error;
		}

		auto info = std::make_pair(true, &bytecode);

		if (lua_load(lua_state, lua_reader, &info, "scriptname", "b") != 0)
			return lua_tostring(lua_state, -1);

		try {
			luabind::call_function<void>(luabind::object(luabind::from_stack(lua_state, -1)));
			lua_pop(lua_state, 1);
		}
		catch (std::exception error_exception) {
			//std::string compilation_error;//(lua_tostring(lua_state, -1));
			report_errors(std::string(error_exception.what()));
			return compilation_error;
		}

		//if (lua_pcall(lua_state, 0, LUA_MULTRET, 0) != LUA_OK) {
		//	
		//}

		report_errors(std::string("Compilation successful."));
		return std::string();
	}
}
