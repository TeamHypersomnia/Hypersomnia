#pragma once

namespace rcon_commands {
	enum class special : unsigned char {
		SHUTDOWN,
		DOWNLOAD_LOGS,

		COUNT
	};
};

using rcon_command_variant = std::variant<
	rcon_commands::special
>;
