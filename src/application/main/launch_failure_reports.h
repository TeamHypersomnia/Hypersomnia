#pragma once
#include <cstddef>
#include <string>

#include "augs/log.h"
#include "augs/log_path_getters.h"
#include "augs/filesystem/file.h"
#include "augs/window_framework/crash_report.h"
#include "augs/window_framework/platform_utils.h"

#include "cmd_line_params.h"

constexpr std::size_t log_entries_in_failure_dialog_v = 6;

inline bool player_sees_failure_dialogs(const cmd_line_params& params) {
	return params.type == app_type::GAME_CLIENT && !params.unit_tests_only && !params.no_open_logs;
}

inline bool game_files_only_next_to_executable() {
	const auto config_name = augs::path_type("default_config.json");

	auto executable_folder = augs::get_executable_path();
	executable_folder.replace_filename("");

	return !augs::exists(config_name) && augs::exists(executable_folder / config_name);
}

inline void start_startup_and_crash_logs(const cmd_line_params& params) {
	augs::set_crash_log_path(::get_crash_log_path());

#if PLATFORM_WEB
	(void)params;
#else
	if (params.type == app_type::GAME_CLIENT && !params.unit_tests_only) {
		::start_startup_log_file(::get_startup_log_path());
	}
#endif
}

inline void show_failure_dialog(const std::string& last_logs, const std::string& failure_log_path) {
	augs::show_error_dialog(
		"Hypersomnia",
		"Hypersomnia stopped because of an error:\n\n" + last_logs + "\nThe whole log was saved to:\n" + failure_log_path
	);
}
