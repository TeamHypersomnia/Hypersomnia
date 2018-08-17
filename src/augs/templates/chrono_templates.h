#pragma once
#include <chrono>
#include <string>

#include "augs/misc/trivially_copyable_tuple.h"

template <class S>
auto lead_zeroes(const S& s, std::size_t count) {
	return s.length() > count ? s : (S(count - s.length(), '0') + s);
}

/* 
	Thanks to https://stackoverflow.com/a/42139394/503776
*/

template <class...Durations, class DurationIn>
auto break_down_durations(DurationIn d) {
	augs::trivially_copyable_tuple<Durations...> retval;

	using discard = int[];
	(void)discard {
		0, (void((
			(std::get<Durations>(retval) = std::chrono::duration_cast<Durations>(d)),
			(d -= std::chrono::duration_cast<DurationIn>(std::get<Durations>(retval)))
			)), 0)...
	};
	return retval;
}

inline std::string format_min_secs_ms(const double seconds) {
	auto clean_duration = break_down_durations<std::chrono::minutes, std::chrono::seconds, std::chrono::milliseconds>(
		std::chrono::duration<double>(seconds)
	);

	return
		lead_zeroes(std::to_string(std::get<0>(clean_duration).count()), 2)
		+ ":"
		+ lead_zeroes(std::to_string(std::get<1>(clean_duration).count()), 2)
		+ ":"
		+ lead_zeroes(std::to_string(std::get<2>(clean_duration).count()), 4)
	;
}

inline std::string format_min_secs(const double seconds) {
	auto clean_duration = break_down_durations<std::chrono::minutes, std::chrono::seconds>(
		std::chrono::duration<double>(seconds)
	);

	return
		lead_zeroes(std::to_string(std::get<0>(clean_duration).count()), 2)
		+ ":"
		+ lead_zeroes(std::to_string(std::get<1>(clean_duration).count()), 2)
	;
}