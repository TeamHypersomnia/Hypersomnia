#include "augs/log.h"
#include "augs/filesystem/file.h"

#include "augs/window_framework/shell.h"
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

	int restart_application(const std::string& arguments) {
		const auto wide = widen(arguments);
		RegisterApplicationRestart(wide.c_str(), 0);

		return EXIT_SUCCESS;
	}
}
#elif PLATFORM_UNIX
#include <cstdlib>

#ifdef __clang__
extern "C" int __cxa_thread_atexit(void (*func)(), void *obj, void *dso_symbol) {
	int __cxa_thread_atexit_impl(void (*)(), void *, void *);
	return __cxa_thread_atexit_impl(func, obj, dso_symbol);
}
#endif

namespace augs {
	int shell(const std::string& s) {
		const auto command = "$SHELL -c \"" + s + "\"";
		LOG("SHELL COMMAND: %x", command);
		return std::system(command.c_str());
	}

	void open_text_editor(const std::string& on_file) {
		const auto full_path = std::filesystem::absolute(augs::path_type(on_file));
		const auto command = augs::path_type("$VISUAL ") += full_path;

		augs::shell(command.string());
	}

	int restart_application(const std::string& arguments) {
		(void)arguments;
		return 42;
	}
}
#else
#error "Unsupported platform!"
#endif

