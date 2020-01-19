#pragma once

struct config_lua_table;

enum class masterserver_udp_command : uint8_t {
	HEARTBEAT, // server to masterserver
	NAT_PUNCH_REQUEST, // client to masterserve
	RESOLVE_EXTERNAL_ADDRESS, // mastserver to client
	SERVER_GOODBYE // server to masterserver
};

void perform_masterserver(const config_lua_table&);
