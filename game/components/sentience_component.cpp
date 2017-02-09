#include "sentience_component.h"

namespace components {
	sentience::sentience() {
		health.enabled = true;
		consciousness.enabled = true;
		personal_electricity.enabled = true;
	}

	bool sentience::meter::is_enabled() const {
		return enabled;
	}

	float sentience::meter::get_maximum_value() const {
		return maximum;
	}

	float sentience::meter::get_value() const {
		return value;
	}

	float sentience::meter::get_ratio() const {
		return value / maximum;
	}

	rgba sentience::calculate_health_color(const float time_pulse_multiplier) const {
		using namespace augs;
		auto hr = health.get_ratio();
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
}