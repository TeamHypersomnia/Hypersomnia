#pragma once
#include <string>

struct lua_State;

namespace augs {
	struct lua_state_wrapper {
		lua_State* raw;

		lua_state_wrapper(const lua_state_wrapper&);
		lua_state_wrapper();
		~lua_state_wrapper();

		std::string get_traceback();

		operator lua_State*();

		void dofile(const std::string& filename);

		/* helper funcs to bind globals to this lua state */
		template<class T>
		void global(std::string name, T& obj) {
			luabind::globals(raw)[name] = &obj;
		}

		template<class T>
		void global_ptr(std::string name, T* obj) {
			luabind::globals(raw)[name] = obj;
		}
	};
}