#pragma once

enum class client_welcome_type : uint8_t {
	UNKNOWN,
	SUPPRESSED,

	WINDOWS,
	LINUX,
	MACOS,

	WEB,
	CRAZYGAMES,

	COUNT
};

inline std::string describe_welcome_type(const client_welcome_type t) {
	switch (t) {
		case client_welcome_type::WINDOWS:
			return " from Windows";
		case client_welcome_type::LINUX:
			return " from Linux";
		case client_welcome_type::MACOS:
			return " from Mac";
		case client_welcome_type::WEB:
			return " from Web";
		case client_welcome_type::CRAZYGAMES:
			return " from CrazyGames";
		default:
			return "";
	}
}
