#pragma once

namespace augs {
	class fixed_delta {
		friend class fixed_delta_timer;
		double fixed_delta_ms;
		unsigned steps_per_second;
	public:
		double in_milliseconds() const;
		double in_seconds() const;
		unsigned get_steps_per_second() const;
	};

	class variable_delta {
		friend class variable_delta_timer;
		double variable_delta_ms = 0.0;
		double interpolation_ratio = 0.0;
		double last_frame_timestamp_seconds = 0.0;
		fixed_delta fixed;
	public:
		fixed_delta get_fixed() const;
		
		double in_milliseconds() const;
		double in_seconds() const;

		double view_interpolation_ratio() const;
		double frame_timestamp_seconds() const;
	};
}
