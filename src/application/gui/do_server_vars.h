#pragma once
#include "application/setups/client/rcon_pane.h"

struct server_vars;
struct server_runtime_info;

/*
	The bullet speed follows the game speed until its slider is moved.
	Moving the game speed again (game_speed_changed) makes it follow once more.
	Returns true if the slider was moved.
*/

bool do_bullet_speed_slider(server_vars& vars, bool game_speed_changed);

/*
	Call right after the respective slider.
*/

void do_game_speed_tooltip();
void do_bullet_speed_tooltip();

void do_server_vars(
	server_vars& vars,
	server_vars& last_saved_vars,
	rcon_pane pane = rcon_pane::ARENAS,
	const server_runtime_info* runtime_info = nullptr
);
