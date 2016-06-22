#pragma once
#include "cosmos.h"
#include "augs/scripting/lua_state_wrapper.h"

#include "game/scene_managers/scene_manager.h"
#include "misc/performance_timer.h"
#include "game/entropy_player.h"

class multiverse {
	scene_manager current_scene_manager;

public:
	augs::performance_timer profile;

	multiverse();

	window::glwindow game_window;

	bool clear_window_inputs_once = true;

	cosmos<testbed> main_cosmos;
	augs::lua_state_wrapper lua;

	void set_scene_manager(std::unique_ptr<scene_manager> builder);
	void build_scene();

	void call_window_script(std::string filename);

	void load_resources();

	void configure_scripting();

	void main_game_loop();

	void consume_camera_render_requests();
};