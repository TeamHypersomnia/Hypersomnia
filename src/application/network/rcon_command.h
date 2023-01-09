#pragma once
#include "application/setups/server/server_vars.h"
#include "game/modes/mode_commands/match_command.h"

namespace rcon_commands {
	enum class special : unsigned char {
		SHUTDOWN,
		RESTART,
		DOWNLOAD_LOGS,

		COUNT
	};
};

using rcon_command_variant = std::variant<
	std::monostate,
	match_command,
	rcon_commands::special,
	server_solvable_vars,
	server_vars
>;
