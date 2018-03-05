#pragma once
#include <ctime>
#include <string>

namespace augs {
	struct date_time {
		// GEN INTROSPECTOR struct augs::timestamp
		std::time_t t;
		// END GEN INTROSPECTOR

		date_time();

		std::string get_stamp() const;
		std::string get_readable() const;
		std::string how_long_ago() const;
	};
}