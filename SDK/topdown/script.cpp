#include "script.h"

extern "C" {
#include <lua\lua.h>
#include <lua\lauxlib.h>
}

script::script() : needs_reloading(false), is_associated_string_filename(false) {

}

void script::associate_string(const std::string& str) {
	associated_string = str;
	is_associated_string_filename = false;
	needs_reloading = true;
}

void script::associate_filename(const std::string& str) {
	associated_string = str;
	is_associated_string_filename = true;
	needs_reloading = true;
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

std::string script::compile(lua_State* lua_state) {
	if (needs_reloading || is_associated_string_filename) {
		int result = LUA_OK;

		if (is_associated_string_filename)
			result = luaL_loadfile(lua_state, associated_string.c_str());
		else
			result = luaL_loadstring(lua_state, associated_string.c_str());

		if (result != LUA_OK) 
			return lua_tostring(lua_state, -1);
		else {
			lua_dump(lua_state, lua_writer, &bytecode);
			lua_pop(lua_state, 1);
		}
	}

	needs_reloading = false;
	return std::string();
}

std::string script::call(lua_State* lua_state) {
	auto info = std::make_pair(true, &bytecode);
	
	if (lua_load(lua_state, lua_reader, &info, "scriptname", "b") != LUA_OK)
		return lua_tostring(lua_state, -1);

	if (lua_pcall(lua_state, 0, LUA_MULTRET, 0) != LUA_OK)
		return lua_tostring(lua_state, -1);

	return std::string();
}
