#pragma once
#include <string>
#include <memory>

struct lua_State;

namespace augs {
	class lua_compilation_result {
	public:
		std::vector<char> bytecode;
		std::string error_message;

		bool is_successful() const;
	};

	class lua_execution_result {
	public:
		std::string error_message;
		std::string exception_message;

		std::string open_editor() const;
	};

	class lua_state_raii {
		std::unique_ptr<lua_State, void(*)(lua_State* const)> raw;

		std::string get_error();
		std::string get_stack();
		void call_debug_traceback(const std::string& method_name = "traceback");
	public:
		lua_state_raii();

		operator lua_State*();

		void dofile_and_report_errors(const std::string& filename);

		lua_compilation_result compile_file(const std::string& filename);
		lua_compilation_result compile_script(const std::string& script);

		lua_execution_result execute(std::vector<char> bytecode);

		/* helper funcs to bind globals to this lua state */
		template <class T>
		void global(const std::string& name, T& obj) {
			luabind::globals(raw.get())[name] = &obj;
		}

		template <class T>
		void global_ptr(const std::string& name, T* const obj) {
			luabind::globals(raw.get())[name] = obj;
		}
	};
}