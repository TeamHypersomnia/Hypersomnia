#pragma once
#include "application/setups/client/rcon_pane.h"

struct server_vars;
struct server_runtime_info;

void do_server_vars(
	server_vars& vars,
	server_vars& last_saved_vars,
	rcon_pane pane = rcon_pane::ARENAS,
	const server_runtime_info* runtime_info = nullptr
);
