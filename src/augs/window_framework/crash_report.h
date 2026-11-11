#pragma once
#include "augs/filesystem/path_declaration.h"

namespace augs {
#if PLATFORM_WINDOWS
	constexpr bool crash_report_saves_the_log_v = true;
#else
	constexpr bool crash_report_saves_the_log_v = false;
#endif

	void report_crashes(bool open_log_in_notepad);
	void set_error_log_path(const path_type& path);
	void stop_reporting_crashes();
}
