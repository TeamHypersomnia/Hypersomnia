#include "script.h"

extern "C" {
#include <lua\lua.h>
#include <lua\lauxlib.h>
}

script::script() : needs_reloading(false), is_associated_string_filename(false), reload_scene_when_modified(true) {

}

void script::associate_string(const std::string& str) {
	associated_string = str;
	is_associated_string_filename = false;
	needs_reloading = true;
}

script::reloader script::script_reloader;

std::vector<script*> script::reloader::get_modified_script_files() {
	std::vector<script*> output;

	if (WaitForSingleObjectEx(changes.GetWaitHandle(), 0, false) == WAIT_OBJECT_0) {
		if (changes.CheckOverflow()) {
			throw std::exception("file change command buffer overflow");
		}
		else {
			DWORD dwAction;
			CStringW wstrFilename;
			
			while (changes.Pop(dwAction, wstrFilename)) {
				if (dwAction == FILE_ACTION_MODIFIED) {
					auto it = filename_to_script.find(std::wstring(wstrFilename));

					if(it != filename_to_script.end()) 
						output.push_back((*it).second);
				}
			}
		}
	}

	return output;
}

void script::reloader::add_directory(std::wstring& wdir, bool subtree) {
	//std::wstring wdir(directory.begin(), directory.end());
	changes.AddDirectory(wdir.c_str(), subtree, FILE_NOTIFY_CHANGE_LAST_WRITE);
}

void script::associate_filename(const std::string& str, bool register_for_file_reloading) {
	if (is_associated_string_filename) 
		script_reloader.filename_to_script.erase(std::wstring(associated_string.begin(), associated_string.end()));

	if (register_for_file_reloading) 
		script_reloader.filename_to_script[std::wstring(str.begin(), str.end())] = this;

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

void script::set_out_of_date() {
	needs_reloading = true;
}

std::string script::compile(lua_State* lua_state) {
	if (needs_reloading) {
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
	auto compilation_error = compile(lua_state);
	if (!compilation_error.empty()) return compilation_error;

	auto info = std::make_pair(true, &bytecode);
	
	if (lua_load(lua_state, lua_reader, &info, "scriptname", "b") != LUA_OK)
		return lua_tostring(lua_state, -1);

	if (lua_pcall(lua_state, 0, LUA_MULTRET, 0) != LUA_OK)
		return lua_tostring(lua_state, -1);

	return std::string();
}
