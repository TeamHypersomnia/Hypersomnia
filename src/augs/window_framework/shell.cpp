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

	int restart_application(const std::string& executable, const std::string& arguments) {
		if (augs::spawn_detached_process(executable, arguments)) {
			return EXIT_SUCCESS;
		}

		return EXIT_FAILURE;
	}
}
#elif PLATFORM_UNIX
#include <cstdlib>

namespace augs {
	int shell(const std::string& s) {
		const auto command = "$SHELL -c \"" + s + "\"";
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

	int restart_application(const std::string& executable, const std::string& arguments) {
		if (augs::spawn_detached_process(executable, arguments)) {
			return EXIT_SUCCESS;
		}

		return EXIT_FAILURE;
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

