#include <ctime>
#include <iomanip>
#include <sstream>
#include <filesystem>

#include "augs/string/typesafe_sprintf.h"
#include "augs/misc/time_utils.h"
#include "3rdparty/date.h"

#if PLATFORM_WINDOWS
#include <Windows.h>
#endif

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

#if defined(__clang__) && defined(PLATFORM_UNIX) 
#if PLATFORM_MACOS
// To be removed once macos updates to AppleClang 14
augs::date_time::date_time(
	const augs::file_time_type& tp
) : 
	date_time(augs::file_time_type::clock::to_time_t(tp)) 
{
}
#else
augs::date_time::date_time(
	const augs::file_time_type& tp
) : 
	date_time(std::chrono::time_point_cast<std::chrono::system_clock::duration>(std::chrono::file_clock::to_sys(tp))) 
	{
}
#endif
#else

static time_t filetime_to_timet(FILETIME const& ft) {
    ULARGE_INTEGER ull;
    ull.LowPart = ft.dwLowDateTime;
    ull.HighPart = ft.dwHighDateTime;
    return ull.QuadPart / 10000000ULL - 11644473600ULL;
}

augs::date_time::date_time(
	const augs::file_time_type& input_filetime
) {
	FILETIME ft;
	std::memcpy(&ft, &input_filetime, sizeof(FILETIME));
	t = filetime_to_timet(ft);
}
#endif

std::string augs::date_time::format_time_point_utc(const std::chrono::system_clock::time_point& tp) {
	std::ostringstream o;
	const auto ttp = std::chrono::time_point_cast<std::chrono::microseconds>(tp);

	{
		using namespace date;
		o << format("%F %T", ttp);
	}
	return o.str() + " UTC";
}

#if PLATFORM_WINDOWS
#define timegm _mkgmtime
#endif

auto convert_string_to_time_t(const std::string &utc_timestamp) {
    std::istringstream ss(utc_timestamp);
    std::tm tm = {};

    ss >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S");

    if (ss.fail()) {
		return std::chrono::system_clock::now();
    }

    const auto seconds = std::chrono::seconds(timegm(&tm));

    // Parse fractional seconds (if present)
    if (ss.peek() == '.') {
        int fractional_seconds;
        ss.ignore();
        ss >> fractional_seconds;

        return std::chrono::system_clock::from_time_t(seconds.count()) + std::chrono::microseconds(fractional_seconds);
    } 

    return std::chrono::system_clock::from_time_t(seconds.count());
}

augs::date_time augs::date_time::from_utc_timestamp(const std::string& timestamp) {
	return augs::date_time(convert_string_to_time_t(timestamp));
}

std::string augs::date_time::get_utc_timestamp() {
	return format_time_point_utc(std::chrono::system_clock::now());
}

/* 
	This sometimes parses wrongly by a microsecond. 
	We've also decided that for improved security,
	we'll just parse the ints by hand (by typesafe_sscanf) as we know the format upfront,
	so we don't really need to use a complex parsing procedure for now.
*/

#if 0
std::optional<std::chrono::system_clock::time_point> augs::date_time::from_utc_timestamp(const std::string& str) {
	std::istringstream in(str);
	date::sys_time<std::chrono::microseconds> tp;

	{
		in >> date::parse("%F %T", tp);
	}

	if (in.fail() || in.bad()) {
		return std::nullopt;
	}

	return tp;
}
#endif

std::mutex localtime_mutex;

std::string augs::date_time::get_readable_format(const char* fmt) const {
	std::string result;

	{
		std::unique_lock<std::mutex> lock(localtime_mutex);
		std::tm local_time = *std::localtime(&t);
		result = typesafe_sprintf("%x", std::put_time(&local_time, fmt));
	}

	return result;
}

std::string augs::date_time::get_readable_for_file() const {
	return get_readable_format("%y.%m.%d at %H-%M-%S");
}

std::string augs::date_time::get_readable_day_hour() const {
	return get_readable_format("%d-%m-%y %H:%M:%S");
}

std::string augs::date_time::get_readable() const {
	return get_readable_format("%H:%M:%S on %d.%m.%y");
}

std::string augs::date_time::how_long_ago() const {
	return format_how_long_ago(false, seconds_ago());
}

std::string augs::date_time::how_long_ago_brief() const {
	return format_how_long_ago_brief(false, seconds_ago());
}

std::string augs::date_time::how_long_ago_tell_seconds() const {
	return format_how_long_ago(true, seconds_ago());
}

uint64_t augs::date_time::seconds_ago() const {
	return static_cast<uint64_t>(std::difftime(std::time(nullptr), t));
}

double augs::date_time::secs_since_epoch() {
	return std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::system_clock::now().time_since_epoch()).count();
}

std::string augs::date_time::format_how_long_ago(const bool tell_seconds, const uint64_t secs) {
	const auto mins = secs / 60;
	const auto hrs = mins / 60;
	const auto days = hrs / 24;

	if (mins < 1) {
		if (tell_seconds) {
			return typesafe_sprintf("%x seconds ago", secs);
		}

		return "Just now";
	}
	else if (mins == 1) {
		return typesafe_sprintf("A minute ago", mins);
	}
	else if (mins < 60) {
		return typesafe_sprintf("%x minutes ago", mins);
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

std::string augs::date_time::format_how_long_ago_brief(const bool tell_seconds, const uint64_t secs) {
	const auto mins = secs / 60;
	const auto hrs = mins / 60;
	const auto days = hrs / 24;

	if (mins < 1) {
		if (tell_seconds) {
			return typesafe_sprintf("%xs", secs);
		}

		return "now";
	}
	else if (mins < 60) {
		return typesafe_sprintf("%xm", mins);
	}
	else if (hrs < 24) {
		return typesafe_sprintf("%xh", hrs);
	}

	return typesafe_sprintf("%xd", days);
}