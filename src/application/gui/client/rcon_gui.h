#pragma once
#include "application/setups/client/rcon_pane.h"
#include "application/setups/server/server_vars.h"
#include "application/setups/server/rcon_level.h"
#include "game/modes/mode_commands/mode_entropy_structs.h"
#include "augs/log.h"

struct rcon_gui_state {
	rcon_pane active_pane = rcon_pane::ARENAS;
	rcon_level_type level = rcon_level_type::DENIED;

	server_vars last_applied_sv_vars;
	server_vars edited_sv_vars;

	server_runtime_info runtime_info;
	custom_game_commands_string_type custom_commands_text;
	bool request_execute_custom_game_commands = false;

	bool show = false;

	bool applying_sv_vars = false;

	void on_arrived(const server_runtime_info& new_runtime_info) {
		LOG("New runtime info arrived. Arenas: %x", new_runtime_info.arenas_on_disk.size());

		runtime_info = new_runtime_info;
	}

	void on_arrived(const server_vars& new_vars) {
		last_applied_sv_vars = new_vars;
		edited_sv_vars = new_vars;

		applying_sv_vars = false;
	}

	bool escape() {
		if (show) {
			show = false;
			return true;
		}

		return false;
	}
};
