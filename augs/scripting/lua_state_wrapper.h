#pragma once
#include <string>

struct lua_State;

namespace augs {
	struct lua_state_wrapper {
		lua_State* raw;

		explicit lua_state_wrapper(lua_State*);
		lua_state_wrapper(const lua_state_wrapper&) = delete;
		lua_state_wrapper();
		~lua_state_wrapper();

		operator lua_State*();

		bool dofile(const std::string& filename);

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

		// utilities

		// returns lines opened by the editor
		static std::string open_editor(std::string errors);
		std::string get_error_and_stack();
		void call_traceback_that_saves_verbose_log();

		void debug_response();
	};
}