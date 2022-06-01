#include "sentience_component.h"

namespace components {
	bool sentience::is_conscious() const {
		return !when_knocked_out.was_set(); //&& get<consciousness_meter_instance>().is_positive();
	}

	bool sentience::unconscious_but_alive() const {
		return false;//get<health_meter_instance>().is_positive() && !get<consciousness_meter_instance>().is_positive();
	}

	bool sentience::is_dead() const {
		return when_knocked_out.was_set() && !get<health_meter_instance>().is_positive();
	}

	rgba sentience::calc_health_color(const float time_pulse_multiplier) const {
		using namespace augs;
		auto hr = get<health_meter_instance>().get_ratio();
		hr *= 1.f - (0.2f * time_pulse_multiplier);

		rgba health_col;

		health_col.set_hsv(augs::interp(cyan.get_hsv(), red.get_hsv(), (1 - hr)*(1 - hr)));
		health_col.a = 220;
		health_col.b = std::min(health_col.b + 120, 255);
		const auto last_g = health_col.g;
		health_col.g = std::min(int(health_col.g), health_col.b + 45);
		health_col.r = std::min(255, health_col.r + last_g - health_col.g + 10);

		const rgba pulse_target(150, 0, 0, 255);
		const auto red_unit = (health_col.r / 255.f);
		const float pulse_redness_multiplier = red_unit * red_unit * red_unit * red_unit * (1 - time_pulse_multiplier);

		return augs::interp(health_col, pulse_target, pulse_redness_multiplier);
	}
	
	std::optional<rgba> sentience::find_low_health_border(const unsigned timestamp_ms) const {
		if (is_conscious()) {
			const auto& health = get<health_meter_instance>();
			auto hr = health.get_ratio();
			const auto one_less_hr = 1.f - hr;

			const auto pulse_duration = static_cast<int>(1250 - 1000 * (1 - hr));

			if (pulse_duration > 0) {
				const auto time_pulse_ratio = (timestamp_ms % pulse_duration) / static_cast<float>(pulse_duration);

				hr *= 1.f - (0.2f * time_pulse_ratio);

				if (hr < 1.f) {
					const auto alpha_multiplier = one_less_hr * one_less_hr * one_less_hr * one_less_hr * time_pulse_ratio;

					return { { 255, 0, 0, static_cast<rgba_channel>(255 * alpha_multiplier) } };
				}
			}
			else {
				return { { 255, 0, 0, 255 } };
			}
		}

		return std::nullopt;
	}
}