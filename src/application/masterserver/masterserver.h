#pragma once

struct config_lua_table;

enum class masterserver_udp_command : uint8_t {
	HEARTBEAT, // server to masterserver
	PUNCH_THIS_SERVER, // client to masterserver
	TELL_ME_MY_ADDRESS, // client to masterserver
	TELL_ME_MY_ADDRESS_RESPONSE, // masterserver to client
	SERVER_GOODBYE // server to masterserver
};

void perform_masterserver(const config_lua_table&);
