#include <string>
#include <thread>
#include <mutex>

#include "augs/log.h"
#include "augs/math/vec2.h"
#include "augs/app_type.h"

#include "augs/filesystem/file.h"
#include "augs/string/string_templates.h"
#include "augs/log_path_getters.h"
#include "augs/misc/time_utils.h"

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

std::mutex log_mutex;

extern bool log_to_live_file;
extern std::string log_timestamp_format;
app_type current_app_type;

std::string get_path_in_log_files(const std::string& name) {
	return std::string(LOG_FILES_DIR) + "/" + get_preffix_for(current_app_type) + name;
}

std::optional<std::string> find_last_incorrect_exit() {
	const auto co_path = get_crashed_controllably_path();
	const auto last_was_controllable_crash = augs::exists(co_path);

	if (last_was_controllable_crash) {
		const auto en_path = get_ensure_failed_path();
		const auto fa_path = get_exit_failure_path();

		const auto ensure_exists = augs::exists(en_path);
		const auto failure_exists = augs::exists(fa_path);

		if (!ensure_exists && !failure_exists) {
			return std::string();
		}

		const auto recent_one = [&]() {
			if (ensure_exists && failure_exists) {
				return 
					augs::last_write_time(en_path) > augs::last_write_time(fa_path) ?
					en_path :
					fa_path
				;
			}

			if (ensure_exists) {
				return en_path;
			}

			return fa_path;
		}();

		return recent_one;
	}
	else {
		/* Either success or an uncontrollable crash */
		const auto success_exists = augs::exists(get_exit_success_path());

		if (success_exists) {
			return std::nullopt;
		}

		return "";
	}
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

std::string get_crashed_controllably_path() {
	return get_path_in_log_files("crashed_controllably");
}

void mark_as_controlled_crash() {
	augs::save_as_text(get_crashed_controllably_path(), "a");
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
	std::unique_lock<std::mutex> lock(log_mutex);

	init_logs_count = all_entries.size();
}

std::size_t program_log::get_init_logs_count() const {
	std::unique_lock<std::mutex> lock(log_mutex);

	return init_logs_count;
}

std::size_t program_log::get_init_logs_count_nomutex() const {
	return init_logs_count;
}

std::string program_log::get_complete() const {
	std::unique_lock<std::mutex> lock(log_mutex);

	auto logs = std::string();

	for (const auto& e : all_entries) {
		logs += e.text + '\n';
	}

	return logs;
}

void LOG_NOFORMAT(const std::string& s) {
#if ENABLE_LOG 
	std::unique_lock<std::mutex> lock(log_mutex);

	auto lg = [&](const auto& f) {
		program_log::get_current().push_entry({ f });
#if OUTPUT_TO_STDOUT
		std::cout << f << std::endl;
#endif

		if (log_to_live_file) {
			std::ofstream recording_file(get_path_in_log_files("live_debug.txt"), std::ios::out | std::ios::app);
			recording_file << f << std::endl;
		}
	};

	if (log_timestamp_format.empty()) {
		lg(s);
	}
	else {
		lg(augs::date_time().get_readable_format(::log_timestamp_format.c_str()) + s);
	}
#else
	(void)s;
#endif
}