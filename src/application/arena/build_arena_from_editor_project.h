#pragma once
#include <vector>
#include "augs/filesystem/path.h"
#include "game/cosmos/cosmos_common_significant_access.h"
#include "application/arena/scene_entity_to_node_map.h"

struct editor_project;
struct packaged_official_content;
struct editor_node_id;
struct cosmos_solvable_significant;

struct build_arena_input {
	const editor_project& project;
	const game_mode_name_type& override_game_mode;
	const augs::path_type& project_resources_parent_folder;
	const packaged_official_content& official;
	scene_entity_to_node_map* scene_entity_to_node;
	cosmos_solvable_significant* target_clean_round_state;
	const bool for_playtesting;
	const bool editor_preview;
};

template <class A>
void build_arena_from_editor_project(A arena_handle, build_arena_input);
