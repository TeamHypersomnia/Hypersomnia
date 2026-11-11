#include <cstddef>
#include <fstream>
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

#if PLATFORM_UNIX
#include <unistd.h>
#endif

augs::mutex log_mutex;

bool log_to_live_file = false;
std::string log_timestamp_format;
std::string live_log_path;
std::ofstream startup_log_file;
app_type current_app_type;

augs::path_type get_log_file_path(const std::string& name) {
	return LOGS_DIR / (get_preffix_for(current_app_type) + name);
}

std::string get_path_in_log_files(const std::string& name) {
	return get_log_file_path(name).string();
}

std::string get_exit_success_path() {
	return get_path_in_log_files("exit_success_debug_log.txt");
}

std::string get_dumped_log_path() {
	return get_path_in_log_files("dumped_debug_log.txt");
}

augs::path_type get_error_log_path() {
	return get_log_file_path("error_log.txt");
}

augs::path_type get_startup_log_path() {
	return get_log_file_path("startup_log.txt");
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
	startup_log_file.close();
}

void start_startup_log_file(const augs::path_type& path) {
	auto lock = augs::scoped_lock(log_mutex);

	startup_log_file.open(path, std::ios::out | std::ios::trunc);
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

	return get_complete_nomutex();
}

std::string program_log::get_complete_nomutex() const {
	auto logs = std::string();

	for (const auto& e : all_entries) {
		logs += e.text + '\n';
	}

	return logs;
}

std::optional<std::string> program_log::try_get_complete_with(const log_entry& last_entry) {
#if WEB_SINGLETHREAD
	push_entry(last_entry);
	return get_complete_nomutex();
#else
	if (!log_mutex.try_lock()) {
		return std::nullopt;
	}

	push_entry(last_entry);
	auto logs = get_complete_nomutex();
	log_mutex.unlock();

	return logs;
#endif
}

std::string& LOG_THREAD_PREFFIX() {
	thread_local std::string preffix;
	return preffix;
}

bool& LOG_THREAD_MUTED() {
	thread_local bool muted = false;
	return muted;
}

static bool stdout_is_terminal() {
#if PLATFORM_UNIX
	static const bool terminal = ::isatty(STDOUT_FILENO) != 0;
	return terminal;
#else
	return false;
#endif
}

static void log_entry_now(const std::string& s, const bool highlighted) {
#if ENABLE_LOG 
	if (LOG_THREAD_MUTED()) {
		return;
	}

	auto lock = augs::scoped_lock(log_mutex);

	auto lg = [&](const auto& f) {
		program_log::get_current().push_entry({ f });
#if OUTPUT_TO_STDOUT
		if (highlighted && stdout_is_terminal()) {
			std::cout << "\x1b[1;33m" << f << "\x1b[0m" << std::endl;
		}
		else {
			std::cout << f << std::endl;
		}
#endif

		if (log_to_live_file) {
			std::ofstream recording_file(live_log_path, std::ios::out | std::ios::app);
			recording_file << f << std::endl;
		}

		if (startup_log_file.is_open()) {
			startup_log_file << f << std::endl;
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
	(void)highlighted;
#endif
}

void LOG_NOFORMAT(const std::string& s) {
	log_entry_now(s, false);
}

void LOG_HIGHLIGHTED_NOFORMAT(const std::string& s) {
	log_entry_now(s, true);
}