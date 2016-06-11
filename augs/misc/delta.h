#pragma once

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
}
