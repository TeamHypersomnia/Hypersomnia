#pragma once
#include "augs/misc/timing/delta.h"
#include "view/necessary_resources.h"
#include "application/input/input_settings.h"
#include "application/app_intent_type.h"

enum class setup_escape_result {
	IGNORE,
	LAUNCH_INGAME_MENU,
	SWITCH_TO_GAME_GUI,
	JUST_FETCH
};

struct setup_advance_input {
	const augs::delta frame_delta;
	const vec2i& screen_size;
	const input_settings& settings;
};

struct simulation_receiver_settings;
class interpolation_system;
class past_infection_system;
struct network_profiler;

struct client_advance_input {
	const vec2i& screen_size;
	const input_settings& settings;

	const simulation_receiver_settings& simulation_receiver;

	network_profiler& network_performance;

	interpolation_system& interp;
	past_infection_system& past_infection;
};

namespace augs {
	class window;
}

namespace sol {
	class state;
}

struct config_lua_table;

struct perform_custom_imgui_input {
	sol::state& lua;
	augs::window& window;
	const images_in_atlas_map& game_atlas;
	const config_lua_table& config;
};

struct handle_input_before_imgui_input {
	const augs::event::state& common_input_state;
	const augs::event::change e;

	augs::window& window;
};

struct handle_input_before_game_input {
	const app_ingame_intent_map& app_controls;
	const necessary_images_in_atlas_map& sizes_for_icons;

	const augs::event::state& common_input_state;
	const augs::event::change e;

	augs::window& window;
};
