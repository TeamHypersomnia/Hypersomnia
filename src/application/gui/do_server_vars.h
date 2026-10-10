#pragma once
#include <cstdint>
#include "augs/templates/maybe.h"
#include "application/setups/client/rcon_pane.h"

struct server_vars;
struct server_runtime_info;

/*
	The bullet speed follows the game speed until its slider is moved.
	Moving the game speed again (game_speed_changed) makes it follow once more.
	Returns true if the slider was moved.
*/

bool do_bullet_speed_slider(
	const char* label,
	real32 game_speed,
	augs::maybe<real32>& bullet_speed,
	bool game_speed_changed
);

/*
	Call right after the respective slider.
*/

void do_game_speed_tooltip();
void do_bullet_speed_tooltip();

/*
	The server settings come in sections - shown as tabs in the settings,
	and as panes with folded sections in the rcon - see do_server_vars.
*/

enum class server_vars_section : uint8_t {
	ARENA,
	MAP_CYCLE,
	GAMEPLAY,
	SERVER_LIST,
	NETWORK,
	TIME_LIMITS,
	RANKED,
	DEDICATED_SERVER
};

void do_server_vars_section(
	server_vars& vars,
	server_vars& last_saved_vars,
	server_vars_section section,
	const server_runtime_info* runtime_info = nullptr
);

void do_server_vars(
	server_vars& vars,
	server_vars& last_saved_vars,
	rcon_pane pane = rcon_pane::ARENAS,
	const server_runtime_info* runtime_info = nullptr
);
