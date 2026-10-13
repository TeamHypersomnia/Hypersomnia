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
		const std::string& /* custom_title */,
		const std::string& /* default_name */
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
	static std::string shell_escape(const std::string& s) {
		std::string result = "'";
		for (const char c : s) {
			if (c == '\'') {
				result += "'\\''";
			} else {
				result += c;
			}
		}
		result += "'";
		return result;
	}

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

	static std::optional<path_type> run_ranger_dialog(const std::string& ranger_flag) {
		const char* terminal = std::getenv("TERMINAL");
		if (!terminal || !command_exists("ranger")) {
			return std::nullopt;
		}

		const auto temp_result = CACHE_DIR / "last_file_path.txt";
		const auto temp_result_escaped = shell_escape(temp_result.string());

		const std::string cmd = std::string(terminal) + " -e ranger " + ranger_flag + "=" + temp_result_escaped;
		augs::shell(cmd);

		try {
			const auto result = file_to_string(temp_result);
			remove_file(temp_result);

			std::string trimmed = result;
			while (!trimmed.empty() && (trimmed.back() == '\n' || trimmed.back() == '\r')) {
				trimmed.pop_back();
			}

			return trimmed.empty() ? std::nullopt : std::optional<path_type>(trimmed);
		}
		catch (const augs::file_open_error&) {
			return std::nullopt;
		}
	}

	bool window::has_file_dialog() const {
		return command_exists("zenity") || command_exists("kdialog") || (std::getenv("TERMINAL") != nullptr && command_exists("ranger"));
	}

	std::optional<path_type> window::open_file_dialog(
		const std::vector<file_dialog_filter>& filters,
		const std::string& custom_title
	) {
		const auto title = custom_title.empty() ? "Open File" : custom_title;

		if (command_exists("zenity")) {
			std::string cmd = "zenity --file-selection --title=" + shell_escape(title);
			for (const auto& f : filters) {
				cmd += " --file-filter=" + shell_escape(f.description + " | " + f.patterns_joined(" "));
			}
			return run_dialog_command(cmd);
		}

		if (command_exists("kdialog")) {
			std::string filter_str;
			for (const auto& f : filters) {
				if (!filter_str.empty()) filter_str += " ";
				filter_str += f.patterns_joined(" ");
			}
			std::string cmd = "kdialog --getopenfilename . " + shell_escape(filter_str) + " --title " + shell_escape(title);
			return run_dialog_command(cmd);
		}

		if (auto result = run_ranger_dialog("--choosefile")) {
			return result;
		}

		LOG("WARNING! No file dialog tool found (zenity, kdialog, or ranger with $TERMINAL).");
		return std::nullopt;
	}

	std::optional<path_type> window::save_file_dialog(
		const std::vector<file_dialog_filter>& filters,
		const std::string& custom_title,
		const std::string& default_name
	) {
		const auto title = custom_title.empty() ? "Save File" : custom_title;

		if (command_exists("zenity")) {
			std::string cmd = "zenity --file-selection --save --confirm-overwrite --title=" + shell_escape(title);

			if (!default_name.empty()) {
				cmd += " --filename=" + shell_escape(default_name);
			}

			for (const auto& f : filters) {
				cmd += " --file-filter=" + shell_escape(f.description + " | " + f.patterns_joined(" "));
			}
			return run_dialog_command(cmd);
		}

		if (command_exists("kdialog")) {
			std::string filter_str;
			for (const auto& f : filters) {
				if (!filter_str.empty()) filter_str += " ";
				filter_str += f.patterns_joined(" ");
			}
			std::string cmd = "kdialog --getsavefilename " + shell_escape(default_name.empty() ? std::string(".") : default_name) + " " + shell_escape(filter_str) + " --title " + shell_escape(title);
			return run_dialog_command(cmd);
		}

		/* For ranger, use :touch to create the file and select it */
		if (auto result = run_ranger_dialog("--choosefile")) {
			return result;
		}

		LOG("WARNING! No file dialog tool found (zenity, kdialog, or ranger with $TERMINAL).");
		return std::nullopt;
	}

	std::optional<path_type> window::choose_directory_dialog(
		const std::string& custom_title
	) {
		const auto title = custom_title.empty() ? "Choose Directory" : custom_title;

		if (command_exists("zenity")) {
			std::string cmd = "zenity --file-selection --directory --title=" + shell_escape(title);
			return run_dialog_command(cmd);
		}

		if (command_exists("kdialog")) {
			std::string cmd = "kdialog --getexistingdirectory . --title " + shell_escape(title);
			return run_dialog_command(cmd);
		}

		if (auto result = run_ranger_dialog("--choosedir")) {
			return result;
		}

		LOG("WARNING! No directory dialog tool found (zenity, kdialog, or ranger with $TERMINAL).");
		return std::nullopt;
	}

	void window::reveal_in_explorer(const augs::path_type& p) {
		std::string shell_command;

		const char* terminal = std::getenv("TERMINAL");
		if (terminal && command_exists("ranger")) {
			shell_command = std::string(terminal) + " -e ranger --selectfile=" + shell_escape(p.string());
		}
		else {
			const auto parent_dir = p.parent_path();
			shell_command = "xdg-open " + shell_escape(parent_dir.string());
		}

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
#include <array>
#include <cstdio>

namespace augs {
	/*
		The system's own file dialogs through AppleScript - "choose file" or "choose file name" (no filters there).
		The chosen path, or nullopt when cancelled.
	*/

	static std::optional<path_type> run_applescript_dialog(const std::string& chooser, const std::string& custom_title) {
		auto prompt = custom_title;

		for (auto& c : prompt) {
			if (c == '"' || c == '\'' || c == '\\') {
				c = ' ';
			}
		}

		const auto command = "osascript -e 'POSIX path of (" + chooser + " with prompt \"" + prompt + "\")' 2>/dev/null";

		FILE* const pipe = popen(command.c_str(), "r");

		if (pipe == nullptr) {
			LOG("Failed to run osascript for the file dialog.");
			return std::nullopt;
		}

		std::array<char, 4096> buffer;
		std::string result;

		while (fgets(buffer.data(), buffer.size(), pipe) != nullptr) {
			result += buffer.data();
		}

		const bool chosen = pclose(pipe) == 0;

		while (!result.empty() && (result.back() == '\n' || result.back() == '\r')) {
			result.pop_back();
		}

		if (!chosen || result.empty()) {
			return std::nullopt;
		}

		return path_type(result);
	}

	std::optional<path_type> window::open_file_dialog(
		const std::vector<file_dialog_filter>& /* filters */,
		const std::string& custom_title
	) {
		return run_applescript_dialog("choose file", custom_title.empty() ? "Open File" : custom_title);
	}

	std::optional<path_type> window::save_file_dialog(
		const std::vector<file_dialog_filter>& /* filters */,
		const std::string& custom_title,
		const std::string& default_name
	) {
		auto chooser = std::string("choose file name");

		if (!default_name.empty() && default_name.find_first_of("\"'\\") == std::string::npos) {
			chooser += " default name \"" + default_name + "\"";
		}

		return run_applescript_dialog(chooser, custom_title.empty() ? "Save File" : custom_title);
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
