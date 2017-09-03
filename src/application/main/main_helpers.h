#pragma once
#include <functional>

#include "augs/misc/machine_entropy.h"
#include "augs/graphics/renderer.h"

#include "game/enums/input_context_enums.h"

class game_gui_system;
class settings_gui_state;
struct configuration_subscribers;
struct config_lua_table;
struct release_flags;

void switch_between_game_gui_and_back(
	game_intent_vector& intents,
	game_gui_system& gui,
	release_flags& flags
);

void switch_developer_console(
	game_intent_vector& intents,
	bool& flag
);

void switch_weapon_laser(
	game_intent_vector& intents,
	bool& flag
);

void clear_debug_lines(
	game_intent_vector& intents,
	std::vector<augs::renderer::debug_line>& lines
);

void handle_exit_events(
	augs::local_entropy&,
	bool& should_quit
);