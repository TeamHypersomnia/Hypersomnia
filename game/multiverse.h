#pragma once
#include "cosmos.h"
#include "augs/scripting/lua_state_wrapper.h"

#include "game/scene_managers/testbed.h"
#include "misc/performance_timer.h"
#include "game/entropy_player.h"

class multiverse {
	scene_managers::testbed main_cosmos_manager;

public:
	multiverse();

	window::glwindow game_window;

	bool clear_window_inputs_once = true;

	cosmos main_cosmos;
	augs::lua_state_wrapper lua;

	void call_window_script(std::string filename);
	void load_resources();

	void configure_scripting();

	void control();
	void simulate();
	void view() const;
};