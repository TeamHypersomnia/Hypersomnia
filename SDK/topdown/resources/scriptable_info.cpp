#include "stdafx.h"
#include "scriptable_info.h"

#include <fstream>

namespace resources {
	script::script() : needs_recompilation(false), is_associated_string_filename(false), reload_scene_when_modified(true) {

	}

	void script::associate_string(const std::string& str) {
		associated_string = str;
		is_associated_string_filename = false;
		needs_recompilation = true;
	}

	void script::dofile(const std::string& filename) {
		static script my_script;
		my_script.associate_filename(filename, false);
		my_script.call();
	}

	lua_State* script::lua_state = nullptr;

	script::reloader script::script_reloader;
	
	script::reloader::script_entry::script_entry(script* script_ptr) : script_ptr(script_ptr) {}

	std::vector<script*> script::reloader::get_script_files_to_reload() {
		std::vector<script*> output;

		if (WaitForSingleObjectEx(changes.GetWaitHandle(), 0, false) == WAIT_OBJECT_0) {
			if (changes.CheckOverflow()) {
				throw std::runtime_error("file change command buffer overflow");
			}
			else {
				DWORD dwAction;
				CStringW wstrFilename;

				std::vector<CStringW> filenames;

				while (changes.Pop(dwAction, wstrFilename))
					if (dwAction == FILE_ACTION_MODIFIED)
						filenames.push_back(wstrFilename);

				std::sort(filenames.begin(), filenames.end());
				filenames.erase(std::unique(filenames.begin(), filenames.end()), filenames.end());

				for (auto& filename : filenames) {
					auto it = filename_to_script.find(std::wstring(filename));

					if (it != filename_to_script.end()) {
						for (auto script_to_reload : (*it).second.reload_dependants)
							output.push_back(script_to_reload);

						(*it).second.script_ptr->needs_recompilation = true;
					}
				}
			}
		}

		return output;
	}

	script::reloader::reloader() : report_errors(nullptr) {}

	void script::reloader::add_directory(const std::wstring& wdir, bool subtree) {
		//std::wstring wdir(directory.begin(), directory.end());
		changes.AddDirectory(wdir.c_str(), subtree, FILE_NOTIFY_CHANGE_LAST_WRITE);
	}

	void script::associate_filename(const std::string& str) {
		associate_filename(str, true);
	}

	void script::associate_filename(const std::string& str, bool register_for_file_reloading) {
		if (is_associated_string_filename)
			script_reloader.filename_to_script.erase(std::wstring(associated_string.begin(), associated_string.end()));

		if (register_for_file_reloading)
			script_reloader.filename_to_script[std::wstring(str.begin(), str.end())] = reloader::script_entry(this);

		associated_string = str;
		is_associated_string_filename = true;
		needs_recompilation = true;
	}

	void script::add_reload_dependant(script* dependant) {
		if (!is_associated_string_filename) throw std::runtime_error("add_reload_dependant call on script that is not a file");
		script_reloader.filename_to_script[std::wstring(associated_string.begin(), associated_string.end())].reload_dependants.push_back(dependant);
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
			int result = LUA_OK;
			bytecode.clear();

			if (is_associated_string_filename) {
				std::ifstream t(associated_string);
				std::string script_file;

				t.seekg(0, std::ios::end);
				script_file.reserve(t.tellg());
				t.seekg(0, std::ios::beg);

				script_file.assign((std::istreambuf_iterator<char>(t)),
					std::istreambuf_iterator<char>());

				result = luaL_loadstring(lua_state, script_file.c_str());
			}
			else
				result = luaL_loadstring(lua_state, associated_string.c_str());

			if (result != LUA_OK)
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
		if (script_reloader.report_errors != nullptr) {
			if (is_associated_string_filename) {
				(*script_reloader.report_errors) << associated_string << ": ";
			}
			(*script_reloader.report_errors) << errors << std::endl;
		}
	}

	std::string script::call() {
		auto compilation_error = compile();
		if (!compilation_error.empty()) {
			report_errors(compilation_error);
			return compilation_error;
		}

		auto info = std::make_pair(true, &bytecode);

		if (lua_load(lua_state, lua_reader, &info, "scriptname", "b") != LUA_OK)
			return lua_tostring(lua_state, -1);

		if (lua_pcall(lua_state, 0, LUA_MULTRET, 0) != LUA_OK) {
			std::string compilation_error(lua_tostring(lua_state, -1));
			report_errors(compilation_error);
			return compilation_error;
		}

		report_errors(std::string("Compilation succesfull."));
		return std::string();
	}
}
