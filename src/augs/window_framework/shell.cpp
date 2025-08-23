#include "augs/log.h"
#include "augs/filesystem/file.h"

#include "augs/window_framework/shell.h"
#include "augs/window_framework/create_process.h"
#include "augs/string/string_templates.h"
#include "augs/misc/mutex.h"
#include "all_paths.h"

#if PLATFORM_WEB
namespace augs {
	int shell(const std::string&) { return 0; }

	void open_text_editor(const std::string&) {}
}
#elif PLATFORM_WINDOWS
#include <Windows.h>
#undef min
#undef max

namespace augs {
	std::wstring widen(const std::string& s); 

	int shell(const std::string& s) {
		LOG("SHELL COMMAND: %x", s);

		const auto wide = widen(s);
		return static_cast<int>(reinterpret_cast<INT_PTR>(ShellExecute(NULL, NULL, wide.c_str(), NULL, NULL, SW_SHOW)));
	}

	void open_text_editor(const std::string& on_file) {
		augs::shell("\"" + std::filesystem::absolute(augs::path_type(on_file)).string() + "\"");
	}
}
#elif PLATFORM_UNIX
#include <cstdlib>

namespace augs {
	int shell(const std::string& s) {
		LOG("SHELL COMMAND: %x", s);
		return std::system(s.c_str());
	}

	void open_text_editor(const std::string& on_file) {
#if HEADLESS
		(void)on_file;
#else
		const auto full_path = std::filesystem::absolute(augs::path_type(on_file));

#if PLATFORM_MACOS
		const auto command = augs::path_type(typesafe_sprintf("open \"%x\"", full_path));
#else
		const auto command = augs::path_type("$VISUAL ") += full_path;
#endif

		augs::shell(command.string());
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
/*
 * This function exists because std::system takes a "const char*" argument,
 * We want something that we can take std::string by-value.
 */
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
		std::string command =
#if PLATFORM_LINUX
			"xdg-open " + url;
#else
			"open " + url;
#endif
		std::thread th(run_command, command);
		th.detach();
	}
#endif
}

#include "augs/templates/container_templates.h"

namespace augs {
	int restart_application(
		const int previous_argc, 
		const char* const * const previous_argv,
		const std::string& executable, 
		std::vector<std::string> added_arguments
	) {
		auto previous_arguments = std::string();

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

			previous_arguments += arg + " ";

			/* Prevent duplicate flags */
			erase_element(added_arguments, arg);
		}

		auto all_added_arguments = std::string();

		for (const auto& a : added_arguments) {
			all_added_arguments += a + " ";
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
		char username[UNLEN + 1]; // UNLEN is defined in Lmcons.h
		DWORD username_len = UNLEN + 1;
		GetUserNameA(username, &username_len);
		return std::string(username);
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
