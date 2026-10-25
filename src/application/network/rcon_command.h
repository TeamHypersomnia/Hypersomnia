#pragma once
#include "application/setups/server/server_vars.h"
#include "game/modes/mode_commands/match_command.h"

using custom_game_commands_string_type = std::string;

enum class server_maintenance_command : unsigned char {
	SHUTDOWN,
	RESTART,
	CHECK_FOR_UPDATES_NOW,
	REQUEST_RUNTIME_INFO,
	DOWNLOAD_LOGS,
	CLEAR_RUNTIME_PREFS,
	CLOSE_ROUND_1,
	TIMEOUT_ABSENT_PLAYERS,

	COUNT
};

enum class rcon_player_action {
	// GEN INTROSPECTOR enum class rcon_player_action
	KICK,
	BAN,

	COUNT
	// END GEN INTROSPECTOR
};

/*
	Kicks or bans one player. A ban holds the account and the address - see server_bans.h.
	ban_minutes == 0 bans for good.
*/

struct rcon_player_command {
	// GEN INTROSPECTOR struct rcon_player_command
	mode_player_id target;
	rcon_player_action action = rcon_player_action::KICK;
	uint32_t ban_minutes = 0;
	// END GEN INTROSPECTOR

	bool operator==(const rcon_player_command&) const = default;
};

using rcon_command_variant = std::variant<
	std::monostate,
	match_command,
	server_maintenance_command,
	custom_game_commands_string_type,
	rcon_player_command
>;
