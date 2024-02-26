#include "augs/window_framework/window.h"
#include "all_paths.h"
#include "augs/filesystem/file.h"
#include "augs/log.h"
#include "augs/window_framework/shell.h"
#include "augs/window_framework/platform_utils.h"

#if PLATFORM_WINDOWS
#include "augs/window_framework/explorer_utils_winapi.hpp"

#elif PLATFORM_LINUX
#include <thread>

namespace augs {
	static std::optional<path_type> read_chosen_path(const augs::path_type& script_path) {
		const auto& temp_result = CACHE_DIR / "last_file_path.txt";

		try {
			const auto result = file_to_string(temp_result);
			remove_file(temp_result);
			return result;
		}
		catch (const augs::file_open_error&) {
			LOG("Error: %x did not produce %x", script_path, temp_result);
			return std::nullopt;
		}
	}

	static std::optional<path_type> choose_path(const augs::path_type& script_path) {
		if (!augs::exists(script_path)) {
			LOG("WARNING! Could not find the script file: %x.", script_path);
			return std::nullopt;
		}

		augs::shell(script_path);
		return read_chosen_path(script_path);
	}

	std::optional<path_type> window::open_file_dialog(
		const std::vector<file_dialog_filter>& /* filters */,
		const std::string& /* custom_title */
	) {
		return choose_path(DETAIL_DIR / "unix/open_file.local");
	}

	std::optional<path_type> window::save_file_dialog(
		const std::vector<file_dialog_filter>& /* filters */,
		const std::string& /* custom_title */
	) {
		return choose_path(DETAIL_DIR / "unix/save_file.local");
	}

	std::optional<path_type> window::choose_directory_dialog(
		const std::string& /* custom_title */
	) {
		return choose_path(DETAIL_DIR / "unix/choose_directory.local");
	}

	void window::reveal_in_explorer(const augs::path_type& p) {
		const auto script_path = DETAIL_DIR / "unix/reveal_file.local";

		if (!augs::exists(script_path)) {
			LOG("WARNING! Could not find the script file: %x.", script_path);
			return;
		}

		const auto shell_command = typesafe_sprintf("%x \"%x\"", script_path, p.string());

		std::thread([shell_command](){ augs::shell(shell_command); }).detach();
	}

	message_box_button window::retry_cancel(
		const std::string& caption,
		const std::string& text
	) {
		LOG("RETRY CANCEL!!");
		LOG_NVPS(caption, text);
		return message_box_button::CANCEL;
	}
}

#elif PLATFORM_MACOS

namespace augs {
	std::optional<path_type> window::open_file_dialog(
		const std::vector<file_dialog_filter>& /* filters */,
		const std::string& /* custom_title */
	) {
		return std::nullopt;
	}

	std::optional<path_type> window::save_file_dialog(
		const std::vector<file_dialog_filter>& /* filters */,
		const std::string& /* custom_title */
	) {
		return std::nullopt;
	}

	std::optional<path_type> window::choose_directory_dialog(
		const std::string& /* custom_title */
	) {
		return std::nullopt;
	}

	message_box_button window::retry_cancel(
		const std::string& caption,
		const std::string& text
	) {
		LOG("RETRY CANCEL!!");
		LOG_NVPS(caption, text);
		return message_box_button::CANCEL;
	}

	void window::reveal_in_explorer(const augs::path_type& full_path) {
		const auto command = typesafe_sprintf("open -R -a Finder \"%x\"", full_path);
		shell(command);
	}
}

#endif
