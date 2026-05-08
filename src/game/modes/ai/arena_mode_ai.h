#pragma once
#include "game/modes/arena_mode.h"
#include "game/modes/ai/arena_mode_ai_structs.h"

struct cosmos_navmesh;
struct randomization;

arena_ai_result update_arena_mode_ai(
	cosmos& cosm,
	const logic_step step,
	arena_mode_ai_state& ai_state,
	arena_mode_ai_team_state& team_state,
	const arena_mode_ai_arena_meta& arena_meta,
	const entity_id controlled_character_id,
	const mode_player_id& bot_player_id,
	const faction_type bot_faction,
	const money_type money,
	const bool is_ffa,
	const bool is_gun_game,
	xorshift_state& stable_round_rng,
	const difficulty_type difficulty,
	const cosmos_navmesh& navmesh,
	const bool bomb_planted,
	const entity_id bomb_entity,
	pathfinding_context* pathfinding_ctx,
	const bool in_buy_area,
	const bool is_freeze_time,
	const std::size_t bot_index,
	const std::size_t num_bots
);

void update_arena_mode_ai_team(
	cosmos& cosm,
	arena_mode_ai_team_state& team_state,
	const arena_mode_ai_arena_meta& arena_meta,
	std::map<mode_player_id, arena_mode_player>& players,
	const faction_type faction,
	const bool bomb_planted,
	randomization& rng,
	const cosmos_navmesh& navmesh,
	const entity_id bomb_entity,
	pathfinding_context* pathfinding_ctx
);

void post_solve_arena_mode_ai(
	cosmos& cosm,
	const logic_step step,
	arena_mode_ai_state& ai_state,
	const entity_id controlled_character_id,
	const bool is_ffa,
	const bool is_gun_game,
	const bool bomb_planted
);
