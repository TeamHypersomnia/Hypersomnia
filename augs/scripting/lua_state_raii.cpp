extern "C" {
	#include <lua/lualib.h>
}

#include <luabind/luabind.hpp>
#include "augs/log.h"
#include "augs/ensure.h"
#include "augs/filesystem/file.h"
#include "augs/filesystem/directory.h"

#include "augs/scripting/lua_state_raii.h"

namespace augs {
	lua_state_raii::lua_state_raii() 
		: raw(
			luaL_newstate(), 
			[](lua_State* const r) {
				lua_close(r); 
			} 
		)
	{
		luaopen_base(raw.get());
		luaL_openlibs(raw.get());
	}

	lua_state_raii::operator lua_State*() {
		return raw.get();
	}

	std::string lua_state_raii::get_error() {
		lua_State* L = raw.get();
		
		std::string str;

		{
			const auto* p = lua_tostring(L, -1);

			if (p != nullptr) {
				str = p;
			}
		}

		return str.empty() ? "\nFailed to retrieve the lua error!" : ("\n" + str);
	}

	std::string lua_state_raii::get_stack() {
		call_debug_traceback();
		
		lua_State* L = raw.get();

		std::string str;
		
		{
			const auto* p = lua_tostring(L, -1);
			
			if (p != nullptr) {
				str = p;
			}
		}

		return str.empty() ? "\nFailed to retrieve the lua call stack!" : ("\n" + str);
	}

	void lua_state_raii::call_debug_traceback(const std::string& method) {
		lua_State* L = raw.get();
		luaL_traceback(L, L, nullptr, 0);
	}

	std::string lua_execution_result::open_editor() const {
		if (error_message.empty()) {
			return "";
		}

		std::stringstream ss(error_message);
		std::string to;

		std::string full_command = std::string("\"") + 
		getenv("AUG_SCRIPTEDITOR") + "\" ";
		std::string lines;

		int lines_found = 0;

		while (std::getline(ss, to, '\n')) {
			std::stringstream line(to);
			std::string to2;
			line >> to2;
			if (to2.empty()) continue;

			to2.erase(to2.end() - 1);
			if (to2.find(".lua") != std::string::npos) {
				const auto exe_dir = augs::get_executable_directory();
				const auto full_file_path = (exe_dir + "\\" + to2);
				
				lines += full_file_path + "\n";
				full_command += full_file_path + " ";

				++lines_found;
			}
		}

		if (lines_found > 0) {
			CALL_SHELL(full_command);
		}

		return lines;
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
		const auto& bytecode = *static_cast<const std::vector<char>*>(data);
		
		*sz = bytecode.size();
		return bytecode.data();
	}

	void lua_state_raii::dofile_and_report_errors(const std::string& filename) {
		LOG("Calling script %x", filename);
		ensure(augs::file_exists(filename));
		
		const auto compiled = compile_file(filename);
		const bool compilation_failed = compiled.error_message.size() > 0;

		if (compilation_failed) {
			LOG("Lua compilation error (%x): %x", filename, compiled.error_message);
			ensure(!compilation_failed);
			return;
		}
		else {
			const auto executed = execute(compiled.bytecode);

			const bool execution_failed = 
				executed.error_message.size() > 0
				|| executed.exception_message.size() > 0;

			if (execution_failed) {
				LOG("Lua execution error (%x): %x\n%x", filename, executed.error_message, executed.exception_message);
				executed.open_editor();

				ensure(!execution_failed);
			}
		}
	}

	lua_compilation_result lua_state_raii::compile_file(const std::string& filename) {
		return compile_script(augs::get_file_contents(filename));
	}
	
	lua_compilation_result lua_state_raii::compile_script(const std::string& script) {
		lua_compilation_result output;
		
		const auto result = luaL_loadstring(raw.get(), script.c_str());

		if (result != 0) {
			output.error_message = get_error();
		}
		else {
			lua_dump(raw.get(), lua_writer, &output.bytecode, 0);
			lua_pop(raw.get(), 1);
		}

		return output;
	}
	
	lua_execution_result lua_state_raii::execute(std::vector<char> bytecode) {
		lua_execution_result output;

		try {
			if (lua_load(raw.get(), lua_reader, reinterpret_cast<void*>(&bytecode), "scriptname", "b") != 0) {
				output.error_message = get_stack() + get_error();
			}

			luabind::call_function<void>(luabind::object(luabind::from_stack(raw.get(), -1)));
			lua_pop(raw.get(), 1);
		}
		catch (char* e) {
			output.exception_message = typesafe_sprintf("Exception thrown! %x", e);
			output.error_message = get_stack() + get_error();
		}
		catch (std::runtime_error e) {
			output.exception_message = typesafe_sprintf("std::runtime_error thrown: %x", e.what());
			output.error_message = get_stack() + get_error();
		}
		catch (luabind::cast_failed e) {
			output.exception_message = typesafe_sprintf("cast_failed thrown: %x (%x)", e.what(), e.info().name());
			output.error_message = get_stack() + get_error();
		}
		catch (luabind::error e) {
			output.exception_message = typesafe_sprintf("luabind::error thrown: %x", e.what());
			output.error_message = get_stack();
		}

		return output;
	}
}