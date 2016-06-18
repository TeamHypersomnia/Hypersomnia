#pragma once
#include "timer.h"
#include "delta.h"
#include "fixed_delta_timer.h"

namespace augs {
	class simulation_timing {
		timer frame_timer;
		timer frame_timestamper;
		double last_frame_timestamp_seconds = 0.0;
	public:
		simulation_timing();

		fixed_delta_timer delta_timer;

		void configure_fixed_delta(float fps, int max_updates_per_step);

		fixed_delta get_fixed_delta() const;
		variable_delta extract_variable_delta();
	};

}