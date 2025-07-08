#pragma once
#include "game/modes/arena_mode.h"
#include "game/modes/arena_mode_ai_structs.h"

arena_ai_result update_arena_mode_ai(
	const arena_mode::input& in,
	const logic_step step,
	arena_mode_player& player,
	bool is_ffa,
	xorshift_state& stable_round_rng
);

void post_solve_arena_mode_ai(const arena_mode::input& in, arena_mode_player& player, const logic_step step);