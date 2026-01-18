#pragma once
#include "augs/math/vec2.h"
#include "augs/misc/randomization.h"
#include "game/cosmos/entity_id.h"
#include "game/cosmos/logic_step.h"
#include "game/modes/mode_player_id.h"
#include "game/enums/faction_type.h"
#include "game/enums/marker_type.h"

class cosmos;
struct arena_mode_ai_team_state;
struct arena_mode_ai_state;

/*
	Context for behavior process() calls.
	Contains commonly needed data to reduce argument duplication.
	Passed to all process() functions so they all have the same signature.
*/

struct ai_behavior_process_ctx {
	cosmos& cosm;
	const logic_step& step;
	arena_mode_ai_state& ai_state;
	arena_mode_ai_team_state& team_state;
	const mode_player_id& bot_player_id;
	const faction_type bot_faction;
	const entity_id controlled_character_id;
	const vec2 character_pos;
	const real32 dt_secs;
	const real32 global_time_secs;
	randomization& rng;
	const entity_id bomb_entity;
	const bool bomb_planted;
	const bool pathfinding_just_completed;
};
