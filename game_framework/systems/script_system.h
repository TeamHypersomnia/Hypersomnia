#pragma once
#include "stdafx.h"

#include "entity_system/processing_system.h"
#include "../components/scriptable_component.h"


using namespace augs;
using namespace entity_system;

struct lua_State;

extern resources::script* world_reloading_script;
class script_system : public processing_system_templated<components::scriptable> {
public:

	struct lua_state_wrapper {
		lua_State* raw;

		lua_state_wrapper(const lua_state_wrapper&) = delete;
		lua_state_wrapper() : raw(luaL_newstate()) {}
		~lua_state_wrapper() { lua_close(raw); }

		operator lua_State*() {
			return raw;
		}
	};

	static void generate_lua_state(lua_state_wrapper&);

	script_system();
	~script_system();

	template<class T>
	void global(lua_State* lua_state, std::string name, T& obj) {
		luabind::globals(lua_state)[name] = &obj;
	}

	void pass_events(world&, bool);
	void call_loop(world&, bool);

	void process_entities(world&) override;
	void process_events(world&) override;
	void substep(world&) override;
};