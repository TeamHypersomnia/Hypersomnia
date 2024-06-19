#pragma once
#include "game/enums/game_intent_type.h"
#include "application/config_json_table.h"

struct input_pass_result {
	game_intents intents;
	raw_game_motion_vector motions;
	config_json_table viewing_config;
};

