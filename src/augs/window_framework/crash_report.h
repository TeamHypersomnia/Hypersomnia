#pragma once
#include <string>

namespace augs {
	void report_crashes(bool show_dialog);
	void set_crash_log_path(const std::string& path);

	void show_error_dialog(const std::string& title, const std::string& text);
}
