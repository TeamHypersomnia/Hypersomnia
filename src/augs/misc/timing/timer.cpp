#include "augs/misc/timing/timer.h"
#include "augs/misc/typesafe_sprintf.h"

namespace augs {
	timer::timer() {
		reset();
	}

	void timer::reset() {
		ticks = now();
	}

	std::string timer::how_long_ago() const {
		const auto m = get<std::chrono::minutes>();
		const auto mins = static_cast<int>(m);
		const auto hrs = mins / 60;
		const auto days = hrs / 24;

		if (mins < 1) {
			return "A while ago";
		}
		else if (mins == 1) {
			return typesafe_sprintf("A minute ago", mins);
		}
		else if (mins < 60) {
			return typesafe_sprintf("%x mins ago", mins);
		}
		else if (hrs == 1) {
			return typesafe_sprintf("An hour ago", hrs);
		}
		else if (hrs < 24) {
			return typesafe_sprintf("%x hours ago", hrs);
		}
		else if (days == 1) {
			return typesafe_sprintf("Yesterday", hrs);
		}

		return typesafe_sprintf("%x days ago", days);
	}
}