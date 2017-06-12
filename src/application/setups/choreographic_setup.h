#pragma once
#include "setup_base.h"

class game_window;

class choreographic_setup : public setup_base {
public:
	void process(
		
		game_window&,
		viewing_session&
	);
};