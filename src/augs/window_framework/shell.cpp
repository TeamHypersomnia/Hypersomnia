#include "augs/log.h"
#include "augs/filesystem/file.h"

#include "augs/window_framework/shell.h"
#include "augs/window_framework/create_process.h"
#include "augs/string/string_templates.h"

#if PLATFORM_WINDOWS
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
		const auto command = "$SHELL -c '" + s + "'";
		LOG("SHELL COMMAND: %x", command);
		return std::system(command.c_str());
	}

	void open_text_editor(const std::string& on_file) {
		const auto full_path = std::filesystem::absolute(augs::path_type(on_file));

#if PLATFORM_MACOS
		const auto command = augs::path_type(typesafe_sprintf("open \"%x\"", full_path));
#else
		const auto command = augs::path_type("$VISUAL ") += full_path;
#endif

		augs::shell(command.string());
	}
}
#else
#error "Unsupported platform!"
#endif


namespace augs {
#if PLATFORM_WINDOWS
	void open_url(const std::string& url) {
		augs::shell(url);
	}
#elif PLATFORM_LINUX
	void open_url(const std::string& url) {
		std::string command = "xdg-open " + url;
		std::system(command.c_str());
	}
#elif PLATFORM_MACOS
	void open_url(const std::string& url) {
        std::string command = "open " + url;
		std::system(command.c_str());
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
		for (int i = 1; i < previous_argc; ++i) {
			const auto arg = std::string(previous_argv[i]);

			if (arg == "--appimage-path") {
				LOG("Restarting the app as an AppImage. Skipping --keep-cwd and --appimage-path arguments.");
				is_appimage = true;
			}
		}
#endif

		for (int i = 1; i < previous_argc; ++i) {
			const auto arg = std::string(previous_argv[i]);

			if (is_appimage) {
				/* 
					Prevent duplicating the flags given by the AppImage run script. 
				*/

				if (arg == "--keep-cwd") {
					continue;
				}

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
		struct passwd *pw = getpwuid(geteuid());
		if (pw != nullptr) {
			return std::string(pw->pw_name);
		}
		return "";
	}
}

#endif
