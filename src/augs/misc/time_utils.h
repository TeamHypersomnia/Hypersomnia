#pragma once
#include <ctime>
#include <string>
#include <chrono>

namespace augs {
	struct date_time {
		// GEN INTROSPECTOR struct augs::timestamp
		std::time_t t;
		// END GEN INTROSPECTOR

		date_time();
		date_time(const std::time_t& t) : t(t) {}
		date_time(const std::chrono::system_clock::time_point&);

		operator std::time_t() const {
			return t;
		}

		std::string get_stamp() const;
		std::string get_readable() const;

		std::string how_long_ago() const;
		std::string how_long_ago_tell_seconds() const;

	private:
		std::string how_long_ago(bool) const;
	};
}