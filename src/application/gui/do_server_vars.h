#pragma once

struct server_vars;
struct server_solvable_vars;

void do_server_vars(
	server_solvable_vars& vars,
	server_solvable_vars& last_saved_vars
);

void do_server_vars(
	server_vars& vars,
	server_vars& last_saved_vars
);
