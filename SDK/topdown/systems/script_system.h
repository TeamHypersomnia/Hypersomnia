#pragma once

#include "entity_system/processing_system.h"
#include "../components/scriptable_component.h"

using namespace augmentations;
using namespace entity_system;

struct lua_State;

class script_system : public processing_system_templated<components::scriptable> {
public:
	lua_State* lua_state;
	void process_entities(world&) override;
};