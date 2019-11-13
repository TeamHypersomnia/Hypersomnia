#pragma once
#include "application/setups/client/rcon_pane.h"
#include "application/setups/server/server_vars.h"
#include "application/setups/server/rcon_level.h"

struct rcon_gui_state {
	rcon_pane active_pane = rcon_pane::ARENAS;
	rcon_level_type level = rcon_level_type::DENIED;

	server_vars last_applied_sv_vars;
	server_vars edited_sv_vars;

	server_solvable_vars last_applied_sv_solvable_vars;
	server_solvable_vars edited_sv_solvable_vars;

	bool show = false;

	bool applying_sv_vars = false;
	bool applying_sv_solvable_vars = false;

	void on_arrived(const server_vars& new_vars) {
		last_applied_sv_vars = new_vars;
		edited_sv_vars = new_vars;

		applying_sv_vars = false;
	}

	void on_arrived(const server_solvable_vars& new_vars) {
		last_applied_sv_solvable_vars = new_vars;
		edited_sv_solvable_vars = new_vars;

		applying_sv_solvable_vars = false;
	}
};
