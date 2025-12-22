#pragma once
#include "augs/misc/timing/delta.h"
#include "view/necessary_resources.h"
#include "application/input/input_settings.h"
#include "application/app_intent_type.h"
#include "augs/math/camera_cone.h"
#include "application/input/entropy_accumulator.h"
#include "application/nat/nat_type.h"
#include "view/viewables/ad_hoc_in_atlas_map.h"

enum class custom_imgui_result {
	NONE,
	GO_TO_MAIN_MENU,
	GO_TO_PROJECT_SELECTOR,
	RETRY,
	OPEN_SELECTED_PROJECT,

	PLAYTEST_ONLINE,
	QUIT_PLAYTESTING
};

enum class setup_escape_result {
	IGNORE,
	LAUNCH_INGAME_MENU,
	GO_TO_MAIN_MENU,
	JUST_FETCH,

	QUIT_PLAYTESTING
};

struct setup_advance_input {
	const augs::delta frame_delta;
	const vec2i screen_size;
	const input_settings settings;
	const zoom_type nonzoomedout_zoom;

	auto make_accumulator_input() const {
		return entropy_accumulator::input {
			settings,
			vec2(screen_size) / nonzoomedout_zoom
		};
	}
};

struct simulation_receiver_settings;
class interpolation_system;
class past_infection_system;
struct network_profiler;
struct network_info;
struct lag_compensation_settings;

struct server_advance_input {
	const vec2i screen_size;
	const input_settings settings;
	const zoom_type nonzoomedout_zoom;

	const nat_detection_result last_detected_nat;

	network_profiler& network_performance;
	server_network_info& server_stats;

	auto make_accumulator_input() const {
		return entropy_accumulator::input {
			settings,
			vec2(screen_size) / nonzoomedout_zoom
		};
	}
};

struct client_advance_input {
	const augs::delta frame_delta;

	const vec2i& screen_size;
	const input_settings settings;
	const zoom_type nonzoomedout_zoom;

	const simulation_receiver_settings& simulation_receiver;
	const lag_compensation_settings& lag_compensation;

	network_profiler& network_performance;
	network_info& network_stats;

	interpolation_system& interp;
	past_infection_system& past_infection;

	auto make_accumulator_input() const {
		return entropy_accumulator::input {
			settings,
			vec2(screen_size) / nonzoomedout_zoom
		};
	}
};

namespace augs {
	class window;
}

namespace sol {
	class state;
}

struct config_json_table;
class images_in_atlas_map;

#if !HEADLESS
struct perform_custom_imgui_input {
	augs::window& window;
	const images_in_atlas_map& game_atlas;
	const ad_hoc_in_atlas_map& ad_hoc_atlas;
	const necessary_images_in_atlas_map& necessary_images;
	const config_json_table& config;

	const bool demo_replay_mode;
};

struct handle_input_before_imgui_input {
	const augs::event::state& common_input_state;
	const augs::event::change e;

	augs::window& window;
};

struct handle_input_before_game_input {
	const general_gui_intent_map& app_controls;
	const game_intent_map& game_controls;
	const necessary_images_in_atlas_map& sizes_for_icons;

	const augs::event::state& common_input_state;
	const augs::event::change e;

	augs::window& window;
};
#endif
