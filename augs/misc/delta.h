#pragma once

namespace augs {
	struct stepped_timestamp;

	class fixed_delta {
		friend class fixed_delta_timer;
		double fixed_delta_ms = 0;
		unsigned steps_per_second = 0;
		unsigned long long total_steps_passed = 0;
	public:

		template <class Archive>
		void serialize(Archive& ar) {
			ar(
				CEREAL_NVP(fixed_delta_ms),
				CEREAL_NVP(steps_per_second),
				CEREAL_NVP(total_steps_passed)
			);
		}

		double in_milliseconds() const;
		double in_seconds() const;
		unsigned get_steps_per_second() const;

		double total_time_passed_in_seconds() const;
		unsigned long long get_total_steps_passed() const;

		stepped_timestamp get_timestamp() const;
	};

	class variable_delta {
		friend class variable_delta_timer;
		double variable_delta_ms = 0.0;
		double interpolation_ratio = 0.0;
		fixed_delta fixed;
	public:
		fixed_delta get_fixed() const;
		
		double in_milliseconds() const;
		double in_seconds() const;

		double view_interpolation_ratio() const;
		double total_time_passed_in_seconds() const;
	};
}
