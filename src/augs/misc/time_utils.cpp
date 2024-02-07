#include <ctime>
#include <iomanip>
#include <sstream>
#include <filesystem>
#include <mutex>

#include "augs/string/typesafe_sprintf.h"
#include "augs/misc/time_utils.h"

#if !PLATFORM_WINDOWS
#define USE_OS_TZDB 1
#include "date/tz.h"
#endif

#include "date/date.h"

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

std::string augs::date_time::format_time_point_utc_iso8601(const std::chrono::system_clock::time_point& tp) {
	// Convert system_clock::time_point to utc tm structure
	auto t = std::chrono::system_clock::to_time_t(tp);
	std::tm* utc_tm = std::gmtime(&t); // Using gmtime to convert to UTC time

	// Use stringstream and put_time to format the tm structure to ISO 8601
	std::ostringstream oss;
	oss << std::put_time(utc_tm, "%FT%T") << "Z"; // ISO 8601 format with Z indicating UTC time

	return oss.str();
}

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

std::string augs::date_time::get_utc_timestamp_iso8601() {
	return format_time_point_utc_iso8601(std::chrono::system_clock::now());
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

int64_t augs::date_time::seconds_ago() const {
	return static_cast<int64_t>(std::difftime(std::time(nullptr), t));
}

double augs::date_time::secs_since_epoch() {
	return std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::system_clock::now().time_since_epoch()).count();
}

std::string augs::date_time::format_how_long_ago(const bool tell_seconds, const int64_t secs) {
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


std::string augs::date_time::format_countdown_letters(const int64_t secs) {
	const int seconds = secs % 60;
	const int totalMinutes = secs / 60;
	const int minutes = totalMinutes % 60;
	const int totalHours = totalMinutes / 60;
	const int hours = totalHours % 24;
	const int days = totalHours / 24;

	std::stringstream ss;

	if (days > 0) {
		ss << days << "d ";
	}

	// Include hours if days > 0 or hours > 0. If days > 0 but hours == 0, include "0h "
	if (days > 0 || hours > 0) {
		ss << hours << "h ";
	}

	// Include minutes if hours > 0, days > 0 or minutes > 0. 
	// If days > 0 or hours > 0 but minutes == 0, include "0m "
	if (hours > 0 || days > 0 || minutes > 0) {
		ss << minutes << "m ";
	}

	// Seconds are always included, as they are the smallest unit of time here
	ss << seconds << "s";

	return ss.str();
}

std::string augs::date_time::format_countdown(const int64_t secs) {
    const int seconds = secs % 60;
    const int totalMinutes = secs / 60;
    const int minutes = totalMinutes % 60;
    const int totalHours = totalMinutes / 60;
    const int hours = totalHours % 24;
    const int days = totalHours / 24;

    char buffer[30];

    std::snprintf(buffer, sizeof(buffer), "%02d:%02d:%02d:%02d", days, hours, minutes, seconds);

    return std::string(buffer);
}

std::string augs::date_time::format_how_long_ago_brief(const bool tell_seconds, const int64_t secs) {
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

// Function to get the IANA time zone name based on locationId
std::string getTimeZoneName(const std::string& locationId) {
    if (locationId == "aus") {
        return "Australia/Sydney";
    } else if (locationId == "ru") {
        return "Europe/Moscow";
    } else if (locationId == "de") {
        return "Europe/Berlin";
    } else if (locationId == "us-central") {
        return "America/Chicago";
    } else if (locationId == "pl") {
        return "Europe/Warsaw";
    }

    return ""; // Return an empty string for unknown locationIds
}

#if PLATFORM_WINDOWS

// MSVC supports C++20 chrono 

double augs::date_time::get_secs_until_next_weekend_evening(const std::string& locationId) {
	using clock = std::chrono::system_clock;
    using namespace std::chrono_literals;

    const auto& timeZoneName = getTimeZoneName(locationId);
    if (timeZoneName.empty()) {
        return -1; // Invalid locationId
    }

    try {
        auto zonedTimeNow = std::chrono::zoned_time{timeZoneName, clock::now()};
        auto localTime = zonedTimeNow.get_local_time();
        auto localDayPoint = std::chrono::floor<std::chrono::days>(localTime);
        std::chrono::weekday localWeekday{localDayPoint};

        auto eveningStart = localDayPoint + 19h;
        auto durationUntilStart = eveningStart - localTime;
        auto secondsUntilStart = std::chrono::duration_cast<std::chrono::seconds>(durationUntilStart).count();

        auto twoHoursInSeconds = std::chrono::duration_cast<std::chrono::seconds>(-2h).count();
        if ((localWeekday == std::chrono::Friday || localWeekday == std::chrono::Saturday || localWeekday == std::chrono::Sunday) &&
            secondsUntilStart >= twoHoursInSeconds && secondsUntilStart < 0) {
            return 0.0; // Event is ongoing
        }

        int daysToAdd = 0;
        if (localWeekday < std::chrono::Friday) {
            daysToAdd = (std::chrono::Friday - localWeekday).count();
        } else if (localWeekday == std::chrono::Friday && secondsUntilStart < 0) {
            daysToAdd = 1; // Next is Saturday
        } else if (localWeekday == std::chrono::Saturday && secondsUntilStart < 0) {
            daysToAdd = 1; // Next is Sunday
        } else {
            daysToAdd = (7 - localWeekday.c_encoding() + std::chrono::Friday.c_encoding()) % 7;
        }

        auto nextEventStart = localDayPoint + std::chrono::days(daysToAdd) + 19h;
        auto durationUntilNextEvent = nextEventStart - localTime;
        return std::chrono::duration_cast<std::chrono::seconds>(durationUntilNextEvent).count();
    } catch (const std::exception& e) {
        return -1; // Indicate an error occurred
    }
}
#else
double augs::date_time::get_secs_until_next_weekend_evening(const std::string& locationId) {
    using namespace std::chrono;

    const auto& timeZoneName = getTimeZoneName(locationId);
    if (timeZoneName.empty()) {
        return -1; // Invalid locationId
    }

    auto timeZone = date::locate_zone(timeZoneName);
    auto now = date::make_zoned(timeZone, system_clock::now());
    auto localTime = now.get_local_time();
    auto localDay = date::floor<date::days>(localTime);
    date::weekday localWeekday{localDay};

    auto eveningStart = localDay + hours(19); // 19:00 on the current day
    auto durationUntilStart = eveningStart - localTime;
    auto secondsUntilStart = duration_cast<seconds>(durationUntilStart).count();

    // Check if the event is ongoing
    if ((localWeekday == date::Friday || localWeekday == date::Saturday || localWeekday == date::Sunday) && 
        secondsUntilStart >= -7200 && secondsUntilStart < 0) {
        return 0.0; // Event is ongoing
    }

    // Calculate time until the next weekend evening
    int daysToAdd = 0;
    auto weekdayEncoding = localWeekday.iso_encoding();
    if (weekdayEncoding <= date::Thursday.iso_encoding()) { // From Sunday to Thursday
        daysToAdd = date::Friday.iso_encoding() - weekdayEncoding;
        if (weekdayEncoding == 7) { // Special case for Sunday
            daysToAdd = 5;
        }
    } else if (weekdayEncoding == date::Friday.iso_encoding() && secondsUntilStart < 0) { // Friday after 19:00
        daysToAdd = 1; // Next is Saturday
    } else if (weekdayEncoding == date::Saturday.iso_encoding() && secondsUntilStart < 0) { // Saturday after 19:00
        daysToAdd = 1; // Next is Sunday
    }

    auto nextEventStart = (localDay + date::days(daysToAdd)) + hours(19); // 19:00 on the next event day
    auto durationUntilNextEvent = nextEventStart - localTime;
    return duration_cast<seconds>(durationUntilNextEvent).count();
}
#endif

std::optional<std::string> augs::date_time::format_time_until_weekend_evening(const std::string& location_id) {
	const auto secs = get_secs_until_next_weekend_evening(location_id);

	if (secs == -1) {
		return std::nullopt;
	}

	if (secs == 0) {
		return std::string("");
	}

	return format_countdown_letters(secs);
}
