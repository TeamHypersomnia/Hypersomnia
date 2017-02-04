#include "sentience_component.h"

namespace components {
	sentience::sentience() {
		health.enabled = true;
		consciousness.enabled = true;
		personal_electricity.enabled = true;
	}

	float sentience::meter::ratio() const {
		return value / maximum;
	}

	sentience::meter& sentience::get(const sentience_meter_type t) {
		switch (t) {
		case sentience_meter_type::CONSCIOUSNESS: return consciousness;
		case sentience_meter_type::HEALTH: return health;
		case sentience_meter_type::PERSONAL_ELECTRICITY: return personal_electricity;
		default: ensure(false); return health;
		}
	}

	const sentience::meter& sentience::get(const sentience_meter_type t) const {
		switch (t) {
		case sentience_meter_type::CONSCIOUSNESS: return consciousness;
		case sentience_meter_type::HEALTH: return health;
		case sentience_meter_type::PERSONAL_ELECTRICITY: return personal_electricity;
		default: ensure(false); return health;
		}
	}

	rgba sentience::calculate_health_color(float time_pulse_multiplier) const {
		using namespace augs;
		auto hr = health.ratio();
		hr *= 1.f - (0.2f * time_pulse_multiplier);

		rgba health_col;

		health_col.set_hsv(augs::interp(cyan.get_hsv(), red.get_hsv(), (1 - hr)*(1 - hr)));
		health_col.a = 220;
		health_col.b = std::min(health_col.b + 120, 255);
		auto last_g = health_col.g;
		health_col.g = std::min(int(health_col.g), health_col.b + 45);
		health_col.r = std::min(255, health_col.r + last_g - health_col.g + 10);

		rgba pulse_target(150, 0, 0, 255);
		auto red_unit = (health_col.r / 255.f);
		float pulse_redness_multiplier = red_unit * red_unit * red_unit * red_unit * (1 - time_pulse_multiplier);

		return augs::interp(health_col, pulse_target, pulse_redness_multiplier);
	}
}