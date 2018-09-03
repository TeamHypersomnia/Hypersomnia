#pragma once

class images_in_atlas_map;
struct mode_player_id;
struct mode_entropy;

struct config_lua_table;

struct draw_mode_gui_input {
	mode_entropy& entropy;
	const float game_screen_top;
	const mode_player_id& local_player;
	const images_in_atlas_map& images_in_atlas;
	const config_lua_table& config;
};
