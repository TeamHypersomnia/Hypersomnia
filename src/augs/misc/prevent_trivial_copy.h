#pragma once

namespace augs {
	struct prevent_trivial_copy {
		prevent_trivial_copy() {}

		prevent_trivial_copy(const prevent_trivial_copy&) {}
		prevent_trivial_copy(prevent_trivial_copy&&) {}

		prevent_trivial_copy& operator=(const prevent_trivial_copy&) { return *this; }
		prevent_trivial_copy& operator=(prevent_trivial_copy&&) { return *this; }
	};
}
