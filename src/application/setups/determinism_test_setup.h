#pragma once
#include "setup_base.h"

class determinism_test_setup : public setup_base {
public:
	void process(
		game_window&,
		viewing_session&
	);
};