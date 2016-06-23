#pragma once
#include "timer.h"
#include "delta.h"
#include "fixed_delta_timer.h"

namespace augs {
	class variable_delta_timer {
		timer frame_timer;
	public:
		variable_delta extract_variable_delta(const fixed_delta_timer&);
	};

}