#pragma once
#include "application/setups/server/server_vars.h"
#include "game/modes/mode_commands/match_command.h"

enum class server_maintenance_command : unsigned char {
		SHUTDOWN,
		RESTART,
		CHECK_FOR_UPDATES_NOW,
		REQUEST_RUNTIME_INFO,
		DOWNLOAD_LOGS,

		COUNT
	};

using rcon_command_variant = std::variant<
	std::monostate,
	match_command,
	server_maintenance_command
>;
