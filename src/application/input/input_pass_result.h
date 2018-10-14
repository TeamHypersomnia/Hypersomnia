#pragma once
#include "game/enums/game_intent_type.h"
#include "application/config_lua_table.h"

struct input_pass_result {
	game_intents intents;
	raw_game_motions motions;
	config_lua_table viewing_config;
};

