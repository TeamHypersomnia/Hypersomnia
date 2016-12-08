#pragma once

namespace augs {
	struct stepped_timestamp;

	class delta {
	protected:
		float delta_ms = 0;
	public:
		bool operator==(const delta& b) const {
			return delta_ms == b.delta_ms;
		}

		float in_milliseconds() const;
		float in_seconds() const;
	};

	class fixed_delta : public delta {
		friend class fixed_delta_timer;
		unsigned steps_per_second = 0;
	public:
		fixed_delta(unsigned steps_per_second = 60);

		bool operator==(const fixed_delta& b) const {
			return steps_per_second == b.steps_per_second && delta::operator==(b);
		}

		template <class Archive>
		void serialize(Archive& ar) {
			ar(
				CEREAL_NVP(delta_ms),
				CEREAL_NVP(steps_per_second),
				CEREAL_NVP(total_steps_passed)
			);
		}

		unsigned get_steps_per_second() const;
	};

	class variable_delta : public delta {
		friend class variable_delta_timer;
		float interpolation_ratio = 0.0;
		fixed_delta fixed;
	public:
		fixed_delta get_fixed() const;
		
		float view_interpolation_ratio() const;
		float seconds_after_last_step() const;
	};
}
