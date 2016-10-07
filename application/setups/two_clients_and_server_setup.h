#pragma once
#include "setup_base.h"

class game_window;

class two_clients_and_server_setup : public setup_base {
public:
	void process(game_window&);
};
