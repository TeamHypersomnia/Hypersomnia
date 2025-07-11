#include <cstddef>
#include <string>
#include <thread>
#include <mutex>

#include "augs/log.h"
#include "augs/math/vec2.h"
#include "augs/app_type.h"

#include "augs/filesystem/file.h"
#include "augs/string/string_templates.h"
#include "augs/log_path_getters.h"
#include "augs/misc/date_time.h"
#include "augs/misc/mutex.h"
#include "all_paths.h"

#if PLATFORM_UNIX
#define OUTPUT_TO_STDOUT 1
#elif BUILD_IN_CONSOLE_MODE
#define OUTPUT_TO_STDOUT 1
#else
#define OUTPUT_TO_STDOUT 0
#endif

#define ENABLE_LOG 1

#if OUTPUT_TO_STDOUT
#include <iostream>
#endif

augs::mutex log_mutex;

bool log_to_live_file = false;
std::string log_timestamp_format;
std::string live_log_path;
app_type current_app_type;

std::string get_path_in_log_files(const std::string& name) {
	return (LOGS_DIR / (get_preffix_for(current_app_type) + name)).string();
}

std::string get_ensure_failed_path() {
	return get_path_in_log_files("ensure_failed_debug_log.txt");
}

std::string get_exit_success_path() {
	return get_path_in_log_files("exit_success_debug_log.txt");
}

std::string get_exit_failure_path() {
	return get_path_in_log_files("exit_failure_debug_log.txt");
}

std::string get_dumped_log_path() {
	return get_path_in_log_files("dumped_debug_log.txt");
}

program_log program_log::global_instance = 10000;

program_log::program_log(const unsigned max_all_entries) 
	: max_all_entries(max_all_entries) 
{
}

void program_log::push_entry(const log_entry& new_entry) {
	all_entries.push_back(new_entry);

	if (all_entries.size() > max_all_entries) {
		all_entries.erase(all_entries.begin(), all_entries.begin() + max_all_entries/5);
	}
}

void program_log::mark_last_init_log() {
	auto lock = augs::scoped_lock(log_mutex);

	init_logs_count = all_entries.size();
}

std::size_t program_log::get_init_logs_count() const {
	auto lock = augs::scoped_lock(log_mutex);

	return init_logs_count;
}

std::size_t program_log::get_init_logs_count_nomutex() const {
	return init_logs_count;
}

std::string program_log::get_complete() const {
	auto lock = augs::scoped_lock(log_mutex);

	auto logs = std::string();

	for (const auto& e : all_entries) {
		logs += e.text + '\n';
	}

	return logs;
}

std::string& LOG_THREAD_PREFFIX() {
	thread_local std::string preffix;
	return preffix;
}

void LOG_NOFORMAT(const std::string& s) {
#if ENABLE_LOG 
	auto lock = augs::scoped_lock(log_mutex);

	auto lg = [&](const auto& f) {
		program_log::get_current().push_entry({ f });
#if OUTPUT_TO_STDOUT
		std::cout << f << std::endl;
#endif

		if (log_to_live_file) {
			std::ofstream recording_file(live_log_path, std::ios::out | std::ios::app);
			recording_file << f << std::endl;
		}
	};

	if (log_timestamp_format.empty()) {
		lg(LOG_THREAD_PREFFIX() + s);
	}
	else {
		lg(augs::date_time().get_readable_format(::log_timestamp_format.c_str()) + LOG_THREAD_PREFFIX() + s);
	}
#else
	(void)s;
#endif
}