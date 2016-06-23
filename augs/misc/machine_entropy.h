#pragma once
#include "augs/window_framework/event.h"
#include <vector>

namespace augs {
	struct machine_entropy {
		std::vector<augs::window::event::state> local;
		// here will be remote entropy as well

		machine_entropy& operator+=(const machine_entropy&);
	};
}