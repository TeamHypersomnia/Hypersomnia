#pragma once
#include "setup_base.h"

class game_window;

class director_setup : public setup_base {
public:
	void process(game_window&);
};