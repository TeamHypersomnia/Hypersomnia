#pragma once
#include <string>

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
		lua_State* raw;

		std::string get_error();
		std::string get_stack();
		void call_debug_traceback(const std::string& method_name = "traceback");
	public:

		explicit lua_state_raii(lua_State*);
		lua_state_raii(const lua_state_raii&) = delete;
		lua_state_raii();
		~lua_state_raii();

		operator lua_State*();

		void dofile_and_report_errors(const std::string& filename);

		lua_compilation_result compile_file(const std::string& filename);
		lua_compilation_result compile_script(const std::string& script);

		lua_execution_result execute(std::vector<char> bytecode);

		/* helper funcs to bind globals to this lua state */
		template<class T>
		void global(std::string name, T& obj) {
			luabind::globals(raw)[name] = &obj;
		}

		template<class T>
		void global_ptr(std::string name, T* obj) {
			luabind::globals(raw)[name] = obj;
		}

		bool owns = true;
	};
}