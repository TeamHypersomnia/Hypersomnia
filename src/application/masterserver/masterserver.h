#pragma once

struct config_lua_table;

enum class masterserver_udp_command : uint8_t {
	HEARTBEAT, // server to client
	NAT_PUNCH_REQUEST // client to server
};

void perform_masterserver(const config_lua_table&);
