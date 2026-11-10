#include <sstream>
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
#include "augs/window_framework/xdg_desktop_portal.h"

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

	static std::optional<path_type> run_ranger_dialog(const std::string& ranger_flag) {
		const char* terminal = std::getenv("TERMINAL");
		if (!terminal || !command_exists("ranger")) {
			return std::nullopt;
		}

		const auto temp_result = CACHE_DIR / "last_file_path.txt";
		const auto temp_result_escaped = augs::quoted_for_shell(temp_result.string());

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

	static std::vector<xdg_desktop_portal::file_filter> to_portal_filters(const std::vector<window::file_dialog_filter>& filters) {
		std::vector<xdg_desktop_portal::file_filter> portal_filters;

		for (const auto& f : filters) {
			portal_filters.push_back({ f.description, f.patterns() });
		}

		return portal_filters;
	}

	static std::string get_home_folder_for_dialogs() {
		if (const char* const home = std::getenv("HOME")) {
			return std::string(home) + "/";
		}

		return "";
	}

	std::string window::get_parent_window_for_portal() const {
		auto out = std::ostringstream();
		out << "x11:" << std::hex << window_id;
		return out.str();
	}

	static bool use_portal_file_chooser() {
		if (xdg_desktop_portal::has_file_chooser()) {
			return true;
		}

		LOG("No file chooser in the desktop portal - falling back to zenity, kdialog or ranger.");
		return false;
	}

	bool window::has_file_dialog() const {
		return
			xdg_desktop_portal::has_file_chooser() ||
			command_exists("zenity") ||
			command_exists("kdialog") ||
			(std::getenv("TERMINAL") != nullptr && command_exists("ranger"))
		;
	}

	std::optional<path_type> window::open_file_dialog(
		const std::vector<file_dialog_filter>& filters,
		const std::string& custom_title
	) {
		const auto title = custom_title.empty() ? "Open File" : custom_title;

		if (use_portal_file_chooser()) {
			return xdg_desktop_portal::choose({
				xdg_desktop_portal::file_chooser_mode::OPEN_FILE,
				title,
				to_portal_filters(filters),
				{},
				{},
				get_parent_window_for_portal()
			});
		}

		if (command_exists("zenity")) {
			std::string cmd = "zenity --file-selection --title=" + augs::quoted_for_shell(title);

			if (const auto home = get_home_folder_for_dialogs(); !home.empty()) {
				cmd += " --filename=" + augs::quoted_for_shell(home);
			}

			for (const auto& f : filters) {
				cmd += " --file-filter=" + augs::quoted_for_shell(f.description + " | " + f.patterns_joined(" "));
			}
			return run_dialog_command(cmd);
		}

		if (command_exists("kdialog")) {
			std::string filter_str;
			for (const auto& f : filters) {
				if (!filter_str.empty()) filter_str += " ";
				filter_str += f.patterns_joined(" ");
			}
			const auto home = get_home_folder_for_dialogs();
			const auto start_folder = home.empty() ? std::string(".") : home;

			std::string cmd = "kdialog --getopenfilename " + augs::quoted_for_shell(start_folder) + " " + augs::quoted_for_shell(filter_str) + " --title " + augs::quoted_for_shell(title);
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

		/*
			A bare name would start in the game's working directory - read-only in an AppImage, gone with a reinstall.
		*/

		const auto default_path = [&]() -> std::string {
			if (default_name.empty() || default_name.front() == '/') {
				return default_name;
			}

			if (const auto home = get_home_folder_for_dialogs(); !home.empty()) {
				return home + default_name;
			}

			return default_name;
		}();

		if (use_portal_file_chooser()) {
			const auto suggested = path_type(default_path);

			return xdg_desktop_portal::choose({
				xdg_desktop_portal::file_chooser_mode::SAVE_FILE,
				title,
				to_portal_filters(filters),
				suggested.filename().string(),
				suggested.parent_path(),
				get_parent_window_for_portal()
			});
		}

		if (command_exists("zenity")) {
			std::string cmd = "zenity --file-selection --save --confirm-overwrite --title=" + augs::quoted_for_shell(title);

			if (!default_path.empty()) {
				cmd += " --filename=" + augs::quoted_for_shell(default_path);
			}

			for (const auto& f : filters) {
				cmd += " --file-filter=" + augs::quoted_for_shell(f.description + " | " + f.patterns_joined(" "));
			}
			return run_dialog_command(cmd);
		}

		if (command_exists("kdialog")) {
			std::string filter_str;
			for (const auto& f : filters) {
				if (!filter_str.empty()) filter_str += " ";
				filter_str += f.patterns_joined(" ");
			}
			std::string cmd = "kdialog --getsavefilename " + augs::quoted_for_shell(default_path.empty() ? std::string(".") : default_path) + " " + augs::quoted_for_shell(filter_str) + " --title " + augs::quoted_for_shell(title);
			return run_dialog_command(cmd);
		}

		/* For ranger, use :touch to create the file and select it */
		if (auto result = run_ranger_dialog("--choosefile")) {
			return result;
		}

		LOG("WARNING! No file dialog tool found (zenity, kdialog, or ranger with $TERMINAL).");
		return std::nullopt;
	}

	static void reveal_without_portal(const augs::path_type& p) {
		std::string shell_command;

		const char* terminal = std::getenv("TERMINAL");
		if (terminal && command_exists("ranger")) {
			shell_command = std::string(terminal) + " -e ranger --selectfile=" + augs::quoted_for_shell(p.string());
		}
		else {
			const auto parent_dir = p.parent_path();
			shell_command = "xdg-open " + augs::quoted_for_shell(parent_dir.string());
		}

		augs::shell(shell_command);
	}

	void window::reveal_in_explorer(const augs::path_type& p) {
		std::thread([p]() {
			if (xdg_desktop_portal::can_open_directory()) {
				xdg_desktop_portal::open_directory_of(p);
				return;
			}

			LOG("No OpenDirectory in the desktop portal - falling back to ranger or xdg-open.");
			reveal_without_portal(p);
		}).detach();
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
