#pragma once
#include "setup_base.h"

class choreographic_setup : public setup_base {
public:
	void process(
		
		augs::window&,
		viewing_session&
	);
};