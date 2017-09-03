#include <thread>
#include <mutex>

#include "augs/window_framework/colored_print.h"
#include "augs/window_framework/log_color.h"

extern std::mutex log_mutex;

template<>
void LOG_COLOR(const console_color c, const std::string& f) {
	std::unique_lock<std::mutex> lock(log_mutex);

	program_log::get_current().push_entry({ c, f });

	augs::colored_print(c, f.c_str());
#if LOG_TO_FILE
	std::ofstream recording_file("generated/logs/live_debug.txt", std::ios::out | std::ios::app);
	recording_file << f << std::endl;
#endif
}