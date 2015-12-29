#pragma once
#include "hypersomnia_world.h"
#include "utilities/lua_state_wrapper.h"

#include "utilities/entity_system/overworld.h"

class hypersomnia_overworld : public augs::overworld {
public:
	hypersomnia_overworld();

	window::glwindow game_window;

	hypersomnia_world game_world;
	augs::lua_state_wrapper lua;

	void initialize();

	void call_window_script(std::string filename);

	void configure_scripting();

	void simulate();
};