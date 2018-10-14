#pragma once
#include "augs/misc/timing/delta.h"
#include "application/input/input_settings.h"

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
