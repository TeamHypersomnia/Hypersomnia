#pragma once
#include <variant>
#include "3rdparty/yojimbo/netcode.io/netcode.h"
#include "application/masterserver/server_heartbeat.h"
#include "application/masterserver/nat_traversal_step_payload.h"
#include "application/masterserver/gameserver_commands.h"

struct config_lua_table;
struct server_heartbeat;

namespace masterserver_in {
	using heartbeat = server_heartbeat;

	struct tell_me_my_address { double session_timestamp = 0.0; };
	struct goodbye {};
}

namespace masterserver_out {
	struct tell_me_my_address { 
		double session_timestamp = 0.0;
		netcode_address_t address;
	};
}

using masterserver_request = std::variant<
	masterserver_in::heartbeat,
	masterserver_in::tell_me_my_address,
	masterserver_in::goodbye,

	masterserver_in::nat_traversal_step,
	masterserver_in::stun_result_info
>;

using masterserver_response = std::variant<
	masterserver_out::tell_me_my_address,
	masterserver_out::stun_result_info
>;

void perform_masterserver(const config_lua_table&);
