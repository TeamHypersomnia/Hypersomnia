#include "game_window.h"

vec2i game_window::get_screen_size() const {
	return window.get_screen_size();
}

void game_window::swap_buffers() {
	window.swap_buffers();
}

decltype(augs::machine_entropy::local) game_window::collect_entropy(const bool enable_cursor_clipping) {
	auto result = window.poll_events(enable_cursor_clipping);

	if (clear_window_inputs_once) {
		result.clear();
		clear_window_inputs_once = false;
	}
	
	return result;
}
