#include "stdafx.h"
#include "script.h"
#include "lua_state_wrapper.h"

#include <fstream>

#include "window_framework/window.h"
#include "file.h"

namespace augs {
	script::script(lua_state_wrapper& lua_state) : lua_state(lua_state) {}

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

	bool script::compile() {
		if (needs_recompilation) {
			int result = 0;
			bytecode.clear();

			if (is_associated_string_filename)
				result = luaL_loadstring(lua_state, get_file_contents(associated_string).c_str());
			else
				result = luaL_loadstring(lua_state, associated_string.c_str());

			if (result != 0)
				return false;
			else {
				lua_dump(lua_state, lua_writer, &bytecode, 0);
				lua_pop(lua_state, 1);
			}
		}

		needs_recompilation = false;
		return true;
	}

	bool script::call() {
		if (!compile())
			return false;

		auto info = std::make_pair(true, &bytecode);

		if (lua_load(lua_state, lua_reader, &info, "scriptname", "b") != 0)
			return false;

		luabind::call_function<void>(luabind::object(luabind::from_stack(lua_state, -1)));
		lua_pop(lua_state, 1);

		return true;
	}

}
