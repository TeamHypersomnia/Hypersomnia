#pragma once
#include "setup_base.h"

class game_window;

class determinism_test_setup : public setup_base {
public:
	void process(
		const config_lua_table& cfg, 
		game_window&,
		viewing_session&
	);
};