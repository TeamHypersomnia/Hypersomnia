#pragma once
#include "stdafx.h"

#include "entity_system/processing_system.h"
#include "../components/scriptable_component.h"


using namespace augs;
using namespace entity_system;

struct lua_State;

namespace resources {
	struct lua_state_wrapper;
}

extern resources::script* world_reloading_script;
class script_system : public processing_system_templated<components::scriptable> {

public:
	static void generate_lua_state(resources::lua_state_wrapper&);

	script_system();
	~script_system();

	template<class T>
	static void global(lua_State* lua_state, std::string name, T& obj) {
		luabind::globals(lua_state)[name] = &obj;
	}	
	
	template<class T>
		static void global_ptr(lua_State* lua_state, std::string name, T* obj) {
		luabind::globals(lua_state)[name] = obj;
	}

	void pass_events(world&, bool);
	void call_loop(world&, bool);

	void process_entities(world&) override;
	void process_events(world&) override;
	void substep(world&) override;
};