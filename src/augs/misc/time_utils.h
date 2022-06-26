#pragma once
#include <ctime>
#include <string>
#include <chrono>
#include "augs/filesystem/file_time_type.h"

namespace augs {
	struct date_time {
		// GEN INTROSPECTOR struct augs::timestamp
		std::time_t t;
		// END GEN INTROSPECTOR

		date_time();
		date_time(const std::time_t& t) : t(t) {}
		date_time(const std::chrono::system_clock::time_point&);
		date_time(const file_time_type&);

		operator std::time_t() const {
			return t;
		}

		std::string get_readable() const;
		std::string get_readable_for_file() const;

		uint64_t seconds_ago() const;
		std::string how_long_ago() const;
		std::string how_long_ago_tell_seconds() const;

		static std::string format_how_long_ago(bool tell_seconds, const uint64_t secs);
		static double secs_since_epoch();

		static std::string format_time_point_utc(const std::chrono::system_clock::time_point& tp);
		static std::string get_utc_timestamp();
#if 0
		static std::optional<std::chrono::system_clock::time_point> from_utc_timestamp(const std::string& s);
#endif

	private:
		std::string how_long_ago(bool tell_seconds) const;
	};
}