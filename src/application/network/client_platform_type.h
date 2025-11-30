#pragma once
#include <cstdint>

enum class client_platform_type : uint8_t {
	UNKNOWN,
	SUPPRESSED,

	WINDOWS,
	LINUX,
	MACOS,

	WEB,
	CRAZYGAMES,
	ITCH,

	COUNT
};

inline std::string describe_from_where(const client_platform_type t) {
	switch (t) {
		case client_platform_type::WINDOWS:
			return " from Windows";
		case client_platform_type::LINUX:
			return " from Linux";
		case client_platform_type::MACOS:
			return " from Mac";
		case client_platform_type::WEB:
			return " from Web";
		case client_platform_type::CRAZYGAMES:
			return " from CrazyGames";
		case client_platform_type::ITCH:
			return " from itch.io";
		default:
			return "";
	}
}

inline bool is_browser_platform(const client_platform_type t) {
	switch (t) {
		case client_platform_type::WEB:
		case client_platform_type::ITCH:
		case client_platform_type::CRAZYGAMES:
			return true;
		default:
			return false;
	}
}

inline client_platform_type get_client_platform_type() {
#if WEB_ITCH
		return client_platform_type::ITCH;
#elif WEB_CRAZYGAMES
		return client_platform_type::CRAZYGAMES;
#elif PLATFORM_WEB
		return client_platform_type::WEB;
#elif PLATFORM_WINDOWS
		return client_platform_type::WINDOWS;
#elif PLATFORM_LINUX
		return client_platform_type::LINUX;
#elif PLATFORM_MACOS
		return client_platform_type::MACOS;
#else
		return client_platform_type::UNKNOWN;
#endif
}
