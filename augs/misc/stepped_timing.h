#pragma once
#include "augs/ensure.h"

namespace augs {
	class fixed_delta;

	struct stepped_timestamp {
		unsigned step = 0;

		template <class Archive>
		void serialize(Archive& ar) {
			ar(CEREAL_NVP(step));
		}

		stepped_timestamp operator-(stepped_timestamp b) const;

		float in_seconds(fixed_delta) const;
		float in_milliseconds(fixed_delta) const;
	};

	struct stepped_timeout {
		stepped_timestamp when_started;

		int is_set = false;
		float timeout_duration_ms = 1000.f;

		template <class Archive>
		void serialize(Archive& ar) {
			ar(
				CEREAL_NVP(when_started),

				CEREAL_NVP(is_set),
				CEREAL_NVP(timeout_duration_ms)
				);
		}

		void unset();
		void set(float timeout_duration_ms, stepped_timestamp);
		bool passed(stepped_timestamp, fixed_delta) const;
		bool lasts(stepped_timestamp, fixed_delta) const;
	};

	struct stepped_cooldown {
		stepped_timestamp when_last_fired;

		int ready_to_fire = true;
		float cooldown_duration_ms = 1000.f;

		template <class Archive>
		void serialize(Archive& ar) {
			ar(
				CEREAL_NVP(when_last_fired),

				CEREAL_NVP(ready_to_fire),
				CEREAL_NVP(cooldown_duration_ms)
			);
		}

		stepped_cooldown(float cooldown_duration_ms);

		bool is_ready(stepped_timestamp, fixed_delta t) const;
		bool try_to_fire_and_reset(stepped_timestamp, fixed_delta t);
	};

}
