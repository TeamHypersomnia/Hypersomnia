#include <cstdint>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <filesystem>
#include "augs/misc/mutex.h"

#include "augs/string/typesafe_sprintf.h"
#include "augs/misc/date_time.h"

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
augs::date_time::date_time(
	const augs::file_time_type& tp
) : 
	date_time(std::chrono::time_point_cast<std::chrono::system_clock::duration>(std::chrono::file_clock::to_sys(tp))) 
	{
}
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

augs::mutex localtime_mutex;

std::string augs::date_time::get_readable_format(const char* fmt) const {
	std::string result;

	{
		auto lock = augs::scoped_lock(localtime_mutex);
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

double augs::steady_secs() {
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::duration<double>>(now.time_since_epoch());
    return elapsed.count();
}

double augs::secs_since_epoch() {
	return augs::date_time::secs_since_epoch();
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

	int n = 0;

	if (days > 0) {
		ss << days << "d ";
		++n;
	}

	// Include hours if days > 0 or hours > 0. If days > 0 but hours == 0, include "0h "
	if (days > 0 || hours > 0) {
		ss << hours << "h ";
		++n;
	}

	if (n >= 2) {
		return ss.str();
	}

	// Include minutes if hours > 0, days > 0 or minutes > 0. 
	// If days > 0 or hours > 0 but minutes == 0, include "0m "
	if (hours > 0 || days > 0 || minutes > 0) {
		ss << minutes << "m ";
		++n;
	}

	if (n >= 2) {
		return ss.str();
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
    if (locationId == "au") {
        return "Australia/Sydney";
    } else if (locationId == "ru") {
        return "Europe/Moscow";
    } else if (locationId == "de") {
        return "Europe/Berlin";
    } else if (locationId == "us-central") {
        return "America/Chicago";
    } else if (locationId == "pl") {
        return "Europe/Warsaw";
	} else if (locationId == "ch") {
		return "Europe/Zurich";
	} else if (locationId == "nl") {
		return "Europe/Amsterdam";
	}

    return ""; // Return an empty string for unknown locationIds
}

#if PLATFORM_WINDOWS
#undef min
#undef max
// MSVC supports C++20 chrono 

double augs::date_time::get_secs_until_next_weekend_evening(const std::string& locationId) {
	using namespace std::chrono;
    using clock = system_clock;
    using namespace std::chrono_literals;

    const auto& timeZoneName = getTimeZoneName(locationId);
    if (timeZoneName.empty()) {
        return -1; // Invalid locationId
    }

    try {
        auto zonedTimeNow = zoned_time{timeZoneName, clock::now()};
        auto localTimeSys = zonedTimeNow.get_sys_time(); // Use system time for comparisons
        auto localDayPoint = floor<days>(localTimeSys); // Get the current day point

        // Convert system_clock::time_point to year_month_day to extract the weekday
        auto localYmd = year_month_day{localDayPoint};

        std::array<time_point<clock>, 3> weekendEvenings{
            zoned_time{timeZoneName, localDayPoint + days((Friday - weekday{localYmd}).count()) + 19h}.get_sys_time(),
            zoned_time{timeZoneName, localDayPoint + days((Saturday - weekday{localYmd}).count()) + 19h}.get_sys_time(),
            zoned_time{timeZoneName, localDayPoint + days((Sunday - weekday{localYmd}).count()) + 19h}.get_sys_time(),
        };

        double closestDistance = std::numeric_limits<double>::max();
        for (const auto& evening : weekendEvenings) {
            auto durationUntilEvening = evening - localTimeSys;
            if (durationUntilEvening.count() >= 0) { // Future event
                closestDistance = std::min(closestDistance, static_cast<double>(duration_cast<seconds>(durationUntilEvening).count()));
            } else if (duration_cast<seconds>(-durationUntilEvening) < 2h) { // Ongoing event
                return 0.0;
            }
        }

        return closestDistance;
    } 
	catch (...) {
        return -1;
    }
}
#else

#if PLATFORM_WEB
double web_get_secs_until_next_weekend_evening(const char*);
#endif

double augs::date_time::get_secs_until_next_weekend_evening(const std::string& locationId) {
#if PLATFORM_WEB
	return web_get_secs_until_next_weekend_evening(locationId.c_str());
#endif

#if 0
	(void)locationId;
	return -1.0;
#else
    using namespace std::chrono;

	try {
		const auto& timeZoneName = getTimeZoneName(locationId);
		if (timeZoneName.empty()) {
			return -1; // Invalid locationId
		}

		auto timeZone = date::locate_zone(timeZoneName);
		auto now = date::make_zoned(timeZone, system_clock::now());
		auto localTime = now.get_local_time();
		auto localDayPoint = floor<days>(localTime);

		std::array<system_clock::time_point, 3> weekendEvenings{
			date::make_zoned(timeZone, localDayPoint + days((date::Friday - date::weekday{localDayPoint}).count()) + hours(19)).get_sys_time(),
			date::make_zoned(timeZone, localDayPoint + days((date::Saturday - date::weekday{localDayPoint}).count()) + hours(19)).get_sys_time(),
			date::make_zoned(timeZone, localDayPoint + days((date::Sunday - date::weekday{localDayPoint}).count()) + hours(19)).get_sys_time(),
		};

		double closestDistance = std::numeric_limits<double>::max();
		for (const auto& evening : weekendEvenings) {
			auto durationUntilEvening = evening - system_clock::now();
			if (durationUntilEvening.count() >= 0) { // Future event
				closestDistance = std::min(closestDistance, static_cast<double>(duration_cast<seconds>(durationUntilEvening).count()));
			} else if (duration_cast<seconds>(-durationUntilEvening) < 2h) { // Ongoing event
				return 0.0;
			}
		}

		return closestDistance;
	}
	catch (...) {
		return -1;
	}
#endif
}

#endif

std::optional<std::string> augs::date_time::format_time_until_weekend_evening(const double secs) {
	if (secs == -1) {
		return std::nullopt;
	}

	if (secs == 0) {
		return std::string("");
	}

	return format_countdown_letters(secs);
}

std::optional<std::string> augs::date_time::format_time_until_weekend_evening(const std::string& location_id) {
	const auto secs = get_secs_until_next_weekend_evening(location_id);
	return format_time_until_weekend_evening(secs);
}

#if BUILD_NETWORKING
/* Reliable yojimbo implementation */
void yojimbo_sleep(double);

namespace augs {
	void sleep(double secs) {
		yojimbo_sleep(secs);
	}
}
#else
#include <thread>

namespace augs {
	void sleep(double secs) {
		std::this_thread::sleep_for(std::chrono::duration<double>(secs));
	}
}
#endif

