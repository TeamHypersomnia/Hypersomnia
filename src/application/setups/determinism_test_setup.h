#pragma once
#include "setup_base.h"

class determinism_test_setup : public setup_base {
public:
	void process(
		augs::window&,
		viewing_session&
	);
};