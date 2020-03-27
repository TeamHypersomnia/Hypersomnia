#pragma once
#include <variant>
#include "3rdparty/yojimbo/netcode.io/netcode.h"
#include "application/masterserver/server_heartbeat.h"

struct config_lua_table;
struct server_heartbeat;

namespace masterserver_in {
	using heartbeat = server_heartbeat;

	struct punch_this_server {
		netcode_address_t address;
	};

	struct tell_me_my_address {};
	struct goodbye {};
}

namespace masterserver_out {
	struct tell_me_my_address { 
		netcode_address_t address;
	};
}

using masterserver_request = std::variant<
	masterserver_in::heartbeat,
	masterserver_in::punch_this_server,
	masterserver_in::tell_me_my_address,
	masterserver_in::goodbye
>;

using masterserver_response = std::variant<
	masterserver_out::tell_me_my_address
>;

void perform_masterserver(const config_lua_table&);
