#pragma once
#include "setup_base.h"

class two_clients_and_server_setup : public setup_base {
public:
	void process(
		config_lua_table& cfg, 
		game_window&
	);
};
