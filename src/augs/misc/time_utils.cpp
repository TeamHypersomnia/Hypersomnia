#include <ctime>
#include <iomanip>
#include <sstream>

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
augs::date_time::date_time(
	const augs::file_time_type& tp
) : 
	date_time(augs::file_time_type::clock::to_time_t(tp)) 
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

std::string augs::date_time::format_time_point(const std::chrono::system_clock::time_point& tp) {
	std::ostringstream o;
	const auto ttp = std::chrono::time_point_cast<std::chrono::microseconds>(tp);

	{
		using namespace date;
		o << format("%F %T", ttp);
	}
	return o.str();
}

std::string augs::date_time::get_utc_timestamp() {
	return format_time_point(std::chrono::system_clock::now()) + " UTC";
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

std::string augs::date_time::get_readable_for_file() const {
	std::tm local_time = *std::localtime(&t);
	return typesafe_sprintf("%x", std::put_time(&local_time, "%y.%m.%d at %H-%M-%S"));
}

std::string augs::date_time::get_readable() const {
	std::tm local_time = *std::localtime(&t);
	return typesafe_sprintf("%x", std::put_time(&local_time, "%H:%M:%S on %m.%d.%y"));
}

std::string augs::date_time::how_long_ago() const {
	return format_how_long_ago(false, seconds_ago());
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

		return "A while ago";
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