#pragma once
#include "game_world.h"
#include "augs/lua_state_wrapper.h"

#include "augs/entity_system/overworld.h"

#include <memory>

#include "game/scene_builders/scene_builder.h"
#include "misc/performance_timer.h"

class game_overworld : public augs::overworld {
	std::unique_ptr<scene_builder> current_scene_builder;

public:
	augs::performance_timer profile;

	game_overworld();

	window::glwindow game_window;

	bool clear_window_inputs_once = true;

	game_world main_game_world;
	augs::lua_state_wrapper lua;

	void set_scene_builder(std::unique_ptr<scene_builder> builder);
	void build_scene();

	void call_window_script(std::string filename);

	void configure_scripting();

	void main_game_loop();

	void consume_camera_render_requests();
};