#pragma once
#include "stdafx.h"

#include "entity_system/processing_system.h"
#include "../components/scriptable_component.h"


using namespace augmentations;
using namespace entity_system;

struct lua_State;

class script_system : public processing_system_templated<components::scriptable> {
public:
	lua_State* lua_state;
	script_system();
	~script_system();

	template<class T>
	void global(std::string name, T& obj) {
		luabind::globals(lua_state)[name] = &obj;
	}

	void process_entities(world&) override;
	void process_events(world&) override;
};