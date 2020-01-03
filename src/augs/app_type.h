#pragma once

enum class app_type {
	MASTERSERVER,
	DEDICATED_SERVER,
	GAME_CLIENT
};

extern app_type current_app_type;

inline std::string get_preffix_for(const app_type t) {
	switch (t) {
		case app_type::MASTERSERVER: return "masterserver_";
		case app_type::DEDICATED_SERVER: return "dedi_server_";
		default: return "";
	}
}
