#include "augs/window_framework/window.h"
#include "all_paths.h"
#include "augs/filesystem/file.h"
#include "augs/log.h"
#include "augs/window_framework/shell.h"
#include "augs/window_framework/platform_utils.h"

#if PLATFORM_WINDOWS
#include "augs/window_framework/explorer_utils_winapi.hpp"

#elif PLATFORM_WEB

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
		(void)full_path;
	}
}

#elif PLATFORM_LINUX
#include <thread>
#include <cstdio>
#include <array>

namespace augs {
	static bool command_exists(const std::string& cmd) {
		const auto check_cmd = "command -v " + cmd + " > /dev/null 2>&1";
		return std::system(check_cmd.c_str()) == 0;
	}

	static std::optional<path_type> run_dialog_command(const std::string& command) {
		std::array<char, 4096> buffer;
		std::string result;

		FILE* pipe = popen(command.c_str(), "r");
		if (!pipe) {
			LOG("Failed to run command: %x", command);
			return std::nullopt;
		}

		while (fgets(buffer.data(), buffer.size(), pipe) != nullptr) {
			result += buffer.data();
		}

		const int exit_code = pclose(pipe);
		if (exit_code != 0 || result.empty()) {
			return std::nullopt;
		}

		/* Remove trailing newline */
		while (!result.empty() && (result.back() == '\n' || result.back() == '\r')) {
			result.pop_back();
		}

		return result;
	}

	std::optional<path_type> window::open_file_dialog(
		const std::vector<file_dialog_filter>& filters,
		const std::string& custom_title
	) {
		const auto title = custom_title.empty() ? "Open File" : custom_title;

		if (command_exists("zenity")) {
			std::string cmd = "zenity --file-selection --title=\"" + title + "\"";
			for (const auto& f : filters) {
				cmd += " --file-filter=\"" + f.description + " | *." + f.extension + "\"";
			}
			return run_dialog_command(cmd);
		}

		if (command_exists("kdialog")) {
			std::string filter_str;
			for (const auto& f : filters) {
				if (!filter_str.empty()) filter_str += " ";
				filter_str += "*." + f.extension;
			}
			std::string cmd = "kdialog --getopenfilename . \"" + filter_str + "\" --title \"" + title + "\"";
			return run_dialog_command(cmd);
		}

		LOG("WARNING! Neither zenity nor kdialog found. Cannot show file dialog.");
		return std::nullopt;
	}

	std::optional<path_type> window::save_file_dialog(
		const std::vector<file_dialog_filter>& filters,
		const std::string& custom_title
	) {
		const auto title = custom_title.empty() ? "Save File" : custom_title;

		if (command_exists("zenity")) {
			std::string cmd = "zenity --file-selection --save --confirm-overwrite --title=\"" + title + "\"";
			for (const auto& f : filters) {
				cmd += " --file-filter=\"" + f.description + " | *." + f.extension + "\"";
			}
			return run_dialog_command(cmd);
		}

		if (command_exists("kdialog")) {
			std::string filter_str;
			for (const auto& f : filters) {
				if (!filter_str.empty()) filter_str += " ";
				filter_str += "*." + f.extension;
			}
			std::string cmd = "kdialog --getsavefilename . \"" + filter_str + "\" --title \"" + title + "\"";
			return run_dialog_command(cmd);
		}

		LOG("WARNING! Neither zenity nor kdialog found. Cannot show file dialog.");
		return std::nullopt;
	}

	std::optional<path_type> window::choose_directory_dialog(
		const std::string& custom_title
	) {
		const auto title = custom_title.empty() ? "Choose Directory" : custom_title;

		if (command_exists("zenity")) {
			std::string cmd = "zenity --file-selection --directory --title=\"" + title + "\"";
			return run_dialog_command(cmd);
		}

		if (command_exists("kdialog")) {
			std::string cmd = "kdialog --getexistingdirectory . --title \"" + title + "\"";
			return run_dialog_command(cmd);
		}

		LOG("WARNING! Neither zenity nor kdialog found. Cannot show directory dialog.");
		return std::nullopt;
	}

	void window::reveal_in_explorer(const augs::path_type& p) {
		const auto parent_dir = p.parent_path();
		const auto shell_command = typesafe_sprintf("xdg-open \"%x\"", parent_dir.string());

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
