#pragma once
#include "stdafx.h"
#include "game_framework/resources/lua_state_wrapper.h"
#include "game_framework/game_framework.h"

#include "network/udp.h"

int main() {
	framework::init();

	resources::lua_state_wrapper lua_state;
	lua_state.bind_whole_engine();

	using namespace augs::network;
	udp udp_socket;

	using namespace augs;
	if (network::init()) {
		if (udp_socket.open()) {
			udp_socket.set_blocking(true);

			network::packet sent_packet; 
			 
			sent_packet.write(std::string("hello there"));

			auto result = udp_socket.send(network::ip(27017, network::ip::get_local_ipv4()), sent_packet);
			  
			std::cout << "\nResult: " << result.result << "Bytes: " << result.bytes_transferred << std::endl;
		}
	} 

	lua_state.dofile("init.lua"); 

	framework::deinit();
	return 0;
}  