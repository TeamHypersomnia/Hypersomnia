#pragma once
#include "augs/misc/timer.h"
#include "augs/misc/delta.h"
#include "augs/misc/fixed_delta_timer.h"

namespace augs {
	class variable_delta_timer {
		timer frame_timer;
	public:
		delta extract_variable_delta(
			const delta& fixed_delta, 
			const fixed_delta_timer&
		);
	};
}