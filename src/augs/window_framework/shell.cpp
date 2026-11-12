#include "augs/log.h"
#include "augs/filesystem/file.h"

#include "augs/window_framework/shell.h"
#include "augs/window_framework/create_process.h"
#include "augs/window_framework/platform_utils.h"
#include "augs/string/string_templates.h"
#include "augs/misc/mutex.h"
#include "all_paths.h"

namespace augs {
	static bool text_editor_disabled = false;

	void disable_text_editor() {
		text_editor_disabled = true;
	}
}

#if PLATFORM_WEB
namespace augs {
	int shell(const std::string&) { return 0; }

	void open_text_editor(const path_type&) {}
}
#elif PLATFORM_WINDOWS
#include <Windows.h>
#undef min
#undef max

namespace augs {
	int shell(const std::string& s) {
		/*
			Without the query - a link page's address confirms the link (see settings_gui).
		*/

		LOG("SHELL COMMAND: %x", s.substr(0, s.find('?')));

		const auto wide = widen(s);
		return static_cast<int>(reinterpret_cast<INT_PTR>(ShellExecute(NULL, NULL, wide.c_str(), NULL, NULL, SW_SHOW)));
	}

	void open_text_editor(const path_type& on_file) {
		if (text_editor_disabled) {
			return;
		}

		const auto quoted_path = L"\"" + std::filesystem::absolute(on_file).wstring() + L"\"";
		ShellExecuteW(NULL, NULL, quoted_path.c_str(), NULL, NULL, SW_SHOW);
	}
}
#elif PLATFORM_UNIX
#include <cstdlib>
#include <unistd.h>

namespace augs {
	int shell(const std::string& s) {
		LOG("SHELL COMMAND: %x", s);
		return std::system(s.c_str());
	}

	void open_text_editor(const path_type& on_file) {
		if (text_editor_disabled) {
			LOG("Not opening %x in a text editor.", on_file);
			return;
		}

#if HEADLESS
		(void)on_file;
#else
		const auto full_path = std::filesystem::absolute(on_file);

#if PLATFORM_MACOS
		const auto command = typesafe_sprintf("open %x", augs::quoted_for_shell(full_path.string()));
#else
		const char* const visual = std::getenv("VISUAL");
		const bool started_from_terminal = isatty(STDIN_FILENO) && isatty(STDOUT_FILENO);
		const bool has_text_editor = started_from_terminal && visual != nullptr && visual[0] != '\0';

		const auto command =
			has_text_editor ?
			"$VISUAL " + augs::quoted_for_shell(full_path.string()) :
			"xdg-open " + augs::quoted_for_shell(full_path.string()) + " > /dev/null 2>&1 &"
		;
#endif

		augs::shell(command);
#endif
	}
}
#else
#error "Unsupported platform!"
#endif


#if PLATFORM_WEB

augs::mutex open_url_on_main_lk;
std::string open_url_on_main;

#elif PLATFORM_UNIX

#include <thread>

static void run_command(std::string command) {
	std::system(command.c_str());
}
#endif

namespace augs {
#if PLATFORM_WINDOWS
	void open_url(const std::string& url) {
		augs::shell(url);
	}
#elif PLATFORM_WEB
	void open_url(const std::string& url) {
		auto lock = augs::scoped_lock(open_url_on_main_lk);
		open_url_on_main = url;
	}
#elif PLATFORM_UNIX
	void open_url(const std::string& url) {
		const auto quoted = augs::quoted_for_shell(url);

#if PLATFORM_LINUX
		std::string command = "xdg-open " + quoted;
#else
		std::string command = "open " + quoted;
#endif

		std::thread th(run_command, command);
		th.detach();
	}
#endif
}

#include "augs/templates/container_templates.h"

static std::string quoted_argument(const std::string& argument) {
#if PLATFORM_UNIX
	return augs::quoted_for_shell(argument);
#else
	auto quoted = std::string("\"");
	auto pending_backslashes = std::size_t(0);

	for (const auto character : argument) {
		if (character == '\\') {
			++pending_backslashes;
			continue;
		}

		if (character == '"') {
			quoted.append(pending_backslashes * 2 + 1, '\\');
		}
		else {
			quoted.append(pending_backslashes, '\\');
		}

		quoted += character;
		pending_backslashes = 0;
	}

	quoted.append(pending_backslashes * 2, '\\');
	quoted += '"';

	return quoted;
#endif
}

static bool is_flag(const std::string& argument) {
	return begins_with(argument, "--");
}

namespace augs {
	int restart_application(
		const int previous_argc, 
		const char* const * const previous_argv,
		const std::string& executable, 
		std::vector<std::string> added_arguments
	) {
		auto previous_arguments = std::string();
		auto propagated_arguments = std::vector<std::string>();

		bool is_appimage = false;

#if PLATFORM_LINUX
		bool is_service = false;

		for (int i = 1; i < previous_argc; ++i) {
			const auto arg = std::string(previous_argv[i]);

			if (arg == "--as-service") {
				is_service = true;
			}

			if (arg == "--appimage-path") {
				LOG("Restarting the app as an AppImage. Skipping --appimage-path argument.");
				is_appimage = true;
			}
		}

		if (is_service) {
			const auto launch_flags_path = path_type(LOGS_DIR / "launch.flags");
			lines_to_file(launch_flags_path, added_arguments);

			return EXIT_SUCCESS;
		}
#endif

		for (int i = 1; i < previous_argc; ++i) {
			const auto arg = std::string(previous_argv[i]);

			if (
				arg == "--upgraded-successfully" || 
				arg == "--update-once-now"
			) {
				/* 
					Do not propagate flags meant to be passed 
					for only a single launch. 
				*/

				continue;
			}

			if (is_appimage) {
				/* 
					Prevent duplicating the flags given by the AppImage run script. 
				*/

				if (arg == "--appimage-path") {
					/* Skip the value as well */
					++i;
					continue;
				}
			}

			previous_arguments += ::quoted_argument(arg) + " ";
			propagated_arguments.push_back(arg);
		}

		auto all_added_arguments = std::string();

		for (std::size_t i = 0; i < added_arguments.size(); ++i) {
			const auto& added = added_arguments[i];

			const bool takes_a_value = i + 1 < added_arguments.size() && !::is_flag(added_arguments[i + 1]);
			const bool already_given = ::is_flag(added) && !takes_a_value && found_in(propagated_arguments, added);

			if (!already_given) {
				all_added_arguments += ::quoted_argument(added) + " ";
			}
		}

		if (all_added_arguments.size() > 0) {
			/* Remove trailing space */
			all_added_arguments.pop_back();
		}

		if (augs::spawn_detached_process(executable, previous_arguments + all_added_arguments)) {
			return EXIT_SUCCESS;
		}

		return EXIT_FAILURE;
	}
}

#if PLATFORM_WINDOWS
#include <Lmcons.h>
namespace augs {
	std::string get_user_name() {
		wchar_t username[UNLEN + 1];
		auto username_length_with_terminator = static_cast<DWORD>(UNLEN + 1);

		if (!GetUserNameW(username, &username_length_with_terminator)) {
			return "";
		}

		return wstr_to_utf8(username);
	}
}

#elif PLATFORM_UNIX

#include <unistd.h>
#include <pwd.h>

namespace augs {
	std::string get_user_name() {
		char* login = getlogin();
		if (login != nullptr) {
			return std::string(login);
		}
		login = getenv("USER");
		if (login != nullptr) {
			return std::string(login);
		}
		struct passwd *pw = getpwuid(getuid());
		if (pw != nullptr) {
			return std::string(pw->pw_name);
		}
		return "";
	}
}

#endif
