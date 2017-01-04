#include <luabind/luabind.hpp>

#include "game_window.h"
#include "game/bindings/bind_game_and_augs.h"
#include "augs/log.h"
#include "augs/filesystem/file.h"

game_window::game_window() {
	bind_game_and_augs(lua);
}

rects::wh<int> game_window::get_screen_size() {
	std::unique_lock<std::mutex> lock(lua_mutex);
	return window.get_screen_size();
}

void game_window::swap_buffers() {
	window.swap_buffers();
}

void game_window::call_config_script(const std::string& filename, const std::string& alternative_filename) {
	std::string used_filename = filename;

	if (augs::file_exists(alternative_filename)) {
		used_filename = alternative_filename;
	}

	{
		std::unique_lock<std::mutex> lock(lua_mutex);

		lua.global_ptr("global_gl_window", &window);

		try {
			if (!lua.dofile(used_filename))
				lua.debug_response();
		}
		catch (char* e) {
			LOG("Exception thrown! %x", e);
			lua.debug_response();
		}
		catch (...) {
			LOG("Exception thrown!");
			lua.debug_response();
		}
	}

	window.gl.initialize();
	window.gl.initialize_fbos(get_screen_size());

	config.get_values(*this);
}

game_window::launch_mode game_window::get_launch_mode() {
	switch (static_cast<int>(config.launch_mode)) {
	case 0: return launch_mode::MAIN_MENU; break;
	case 1: return launch_mode::LOCAL; break;
	case 2: return launch_mode::LOCAL_DETERMINISM_TEST; break;
	case 3: return launch_mode::DIRECTOR; break;
	case 4: return launch_mode::ONLY_CLIENT; break;
	case 5: return launch_mode::ONLY_SERVER; break;
	case 6: return launch_mode::CLIENT_AND_SERVER; break;
	case 7: return launch_mode::TWO_CLIENTS_AND_SERVER; break;
	default: return launch_mode::INVALID; break;
	}
}

input_recording_mode game_window::get_input_recording_mode() {
	switch (static_cast<int>(config.input_recording_mode)) {
	case 0: return input_recording_mode::DISABLED; break;
	case 1: return input_recording_mode::LIVE_WITH_BUFFER; break;
	case 2: return input_recording_mode::LIVE; break;
	default: return input_recording_mode::DISABLED; break;
	}
}

decltype(machine_entropy::local) game_window::collect_entropy() {
	auto result = window.poll_events(!config.debug_disable_cursor_clipping);

	if (clear_window_inputs_once) {
		result.clear();
		clear_window_inputs_once = false;
	}
	
	return result;
}

void game_window::get_config_value(const std::string& field, bool& into) {
	std::unique_lock<std::mutex> lock(lua_mutex);

	into = luabind::object_cast<double>(luabind::globals(lua.raw)["config_table"][field]) > 0.0;
}