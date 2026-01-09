#pragma once
#include "game/modes/arena_mode.h"
#include "game/modes/ai/arena_mode_ai_structs.h"

struct cosmos_navmesh;

arena_ai_result update_arena_mode_ai(
	const cosmos& cosm,
	const logic_step step,
	arena_mode_ai_state& ai_state,
	const entity_id controlled_character_id,
	const money_type money,
	const bool is_ffa,
	xorshift_state& stable_round_rng,
	const difficulty_type difficulty,
	const cosmos_navmesh& navmesh
);

void post_solve_arena_mode_ai(
	const cosmos& cosm,
	const logic_step step,
	arena_mode_ai_state& ai_state,
	const entity_id controlled_character_id,
	const bool is_ffa
);
