#include <ctime>
#include <iomanip>
#include <sstream>

#include "augs/string/typesafe_sprintf.h"
#include "augs/misc/time_utils.h"

template<class t>
static std::string leading_zero(const t component) {
	std::stringstream out;
	out << component;
	std::string result = out.str();

	if (result.length() == 1) {
		result = '0' + result;
	}

	return result;
}

augs::date_time::date_time() : date_time(std::time(nullptr)) {}

augs::date_time::date_time(
	const std::chrono::system_clock::time_point& tp
) : 
	date_time(std::chrono::system_clock::to_time_t(tp)) 
{
}

std::string augs::date_time::get_stamp() const {
    std::tm local_time = *std::localtime(&t);

	return	
		"[" 
		+ leading_zero(local_time.tm_mday) + "." 
		+ leading_zero(local_time.tm_mon + 1) + "." 
		+ leading_zero(local_time.tm_year + 1900) + "_" 
		+ leading_zero(local_time.tm_hour) + "." 
		+ leading_zero(local_time.tm_min) + "." 
		+ leading_zero(local_time.tm_sec)
		+ "]"
	;
}

std::string augs::date_time::get_readable() const {
	std::tm local_time = *std::localtime(&t);
	return typesafe_sprintf("%x", std::put_time(&local_time, "%H:%M:%S on %m.%d.%y"));
}

std::string augs::date_time::how_long_ago() const {
	return how_long_ago(false);
}

std::string augs::date_time::how_long_ago_tell_seconds() const {
	return how_long_ago(true);
}

std::string augs::date_time::how_long_ago(const bool tell_seconds) const {
	const auto secs = static_cast<unsigned long long>(std::difftime(std::time(nullptr), t));

	const auto mins = secs / 60;
	const auto hrs = mins / 60;
	const auto days = hrs / 24;

	if (mins < 1) {
		if (tell_seconds) {
			return typesafe_sprintf("%x seconds ago", secs);
		}

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