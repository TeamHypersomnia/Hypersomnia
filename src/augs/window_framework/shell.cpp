#include "augs/filesystem/file.h"

#include "augs/window_framework/shell.h"
#include "augs/templates/string_templates.h"

#if PLATFORM_WINDOWS
#include <Windows.h>
#undef min
#undef max

namespace augs {
	std::wstring widen(const std::string& s); 

	int shell(const std::string& s) {
		LOG("SHELL COMMAND: %x", s);

#if TODO_STRINGS
		const auto wide = widen(s);
		return static_cast<int>(reinterpret_cast<INT_PTR>(ShellExecute(NULL, NULL, wide.c_str(), NULL, NULL, SW_SHOW)));
#else
		return 0;
#endif
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
}
#else
#error "Unsupported platform!"
#endif

namespace augs {
	void open_text_editor(const std::string& on_file) {
		std::string command;

#if PLATFORM_WINDOWS
		command = on_file;
#elif PLATFORM_UNIX
		{
			auto command_with_path = augs::path_type("$VISUAL ");

			{
				const auto full_path = std::experimental::filesystem::system_complete(augs::path_type(on_file));
				command_with_path += full_path;
			}

			command = command_with_path;
		}
#else
#error "Unsupported platform!"
#endif
		augs::shell(command);
	}
}
