#pragma once
#include <variant>
#include "3rdparty/yojimbo/netcode.io/netcode.h"
#include "application/masterserver/server_heartbeat.h"

struct config_lua_table;
struct server_heartbeat;

struct nat_traversal_step_params {
	bool stun_required = false;
	bool send_many = false;
};

namespace masterserver_in {
	using heartbeat = server_heartbeat;

	struct tell_me_my_address {};
	struct goodbye {};

	struct nat_traversal_step { 
		netcode_address_t target_server;
		port_type source_external_port = 0;
		nat_traversal_step_params params;
	};
}

namespace masterserver_out {
	struct tell_me_my_address { 
		netcode_address_t address;
	};

	struct nat_traversal_step { 
		netcode_address_t source_address;
		nat_traversal_step_params params;
	};
}

using masterserver_request = std::variant<
	masterserver_in::heartbeat,
	masterserver_in::tell_me_my_address,
	masterserver_in::goodbye,

	masterserver_in::nat_traversal_step
>;

using masterserver_response = std::variant<
	masterserver_out::tell_me_my_address
>;

struct gameserver_ping_request {
	uint64_t sequence = -1;
};

struct gameserver_ping_response {
	uint64_t sequence = -1;
};

using gameserver_command = std::variant<
	gameserver_ping_request,
	masterserver_out::nat_traversal_step
>;

void perform_masterserver(const config_lua_table&);
