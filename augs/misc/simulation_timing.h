#pragma once
#include "timer.h"
#include "fixed_delta_timer.h"

namespace augs {
	class fixed_delta {
		friend class simulation_timing;
		double fixed_delta_ms;
	public:
		double delta_milliseconds() const;
		double delta_seconds() const;
	};

	class variable_delta {
		friend class simulation_timing;
		double variable_delta_ms = 0.0;
		double fixed_delta_ms = 0.0;
		double interpolation_ratio = 0.0;
		double last_frame_timestamp_seconds = 0.0;
	public:
		double delta_milliseconds() const;
		double delta_seconds() const;

		double fixed_delta_milliseconds() const;
		double view_interpolation_ratio() const;
		double frame_timestamp_seconds() const;
	};
	
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