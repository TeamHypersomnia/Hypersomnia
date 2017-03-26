#pragma once
#include "setup_base.h"

class game_window;

class choreographic_setup : public setup_base {
public:
	void process(
		const config_lua_table& cfg, 
		game_window&
	);
};