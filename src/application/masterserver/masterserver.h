#pragma once
#include "application/config_lua_table.h"

enum class masterserver_udp_command : uint8_t {
	HEARTBEAT, // server to client
	NAT_OPEN_REQUEST // client to server
};

void perform_masterserver(const config_lua_table&);
