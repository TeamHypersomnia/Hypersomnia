#pragma once
struct lua_State;

namespace resources {
	struct lua_state_wrapper {
		lua_State* raw;

		lua_state_wrapper(const lua_state_wrapper&) {
			assert(0);
		}

		lua_state_wrapper() : raw(luaL_newstate()) {}
		~lua_state_wrapper() { lua_close(raw); }

		operator lua_State*() {
			return raw;
		}

		/* binds to the lua_State entire game framework along with augs utilities */
		void bind_whole_engine();

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