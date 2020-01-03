#include <string>
#include <thread>
#include <mutex>

#include "augs/log.h"
#include "augs/math/vec2.h"

#include "augs/filesystem/file.h"
#include "augs/string/string_templates.h"
#include "augs/log_path_getters.h"

#define ENABLE_LOG 1

#if BUILD_IN_CONSOLE_MODE
#include <iostream>
#endif

std::mutex log_mutex;

extern bool log_to_live_file;

std::string get_path_in_log_files(const std::string& name) {
	return std::string(LOG_FILES_DIR) + "/" + get_preffix_for(current_app_type) + name;
}

std::string find_last_incorrect_exit() {
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

	const auto su_exists = augs::exists(get_exit_success_path());

	if (!su_exists) {
		return recent_one;
	}

	const auto last_succ = augs::last_write_time(get_exit_success_path());

	if (last_succ < augs::last_write_time(recent_one)) {
		return recent_one;
	}
	
	return std::string();
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

std::string program_log::get_complete() const {
	std::unique_lock<std::mutex> lock(log_mutex);

	auto logs = std::string();

	for (const auto& e : all_entries) {
		logs += e.text + '\n';
	}

	return logs;
}

void LOG_DIRECT(const std::string& f) {
#if ENABLE_LOG 
	std::unique_lock<std::mutex> lock(log_mutex);

	program_log::get_current().push_entry({ f });

#if BUILD_IN_CONSOLE_MODE
	std::cout << f << std::endl;
#endif

	if (log_to_live_file) {
		std::ofstream recording_file(get_path_in_log_files("live_debug.txt"), std::ios::out | std::ios::app);
		recording_file << f << std::endl;
	}
#else
	(void)f;
#endif
}