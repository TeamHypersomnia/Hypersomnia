#include "augs/filesystem/file.h"

#include "augs/window_framework/shell.h"
#include "augs/templates/string_templates.h"

#if PLATFORM_WINDOWS
#include <Windows.h>
#undef min
#undef max

namespace augs {
	int shell(const std::string& s) {
		const auto wide = to_wstring(s);

		return static_cast<int>(reinterpret_cast<INT_PTR>(ShellExecute(NULL, NULL, wide.c_str(), NULL, NULL, SW_SHOW)));
	}
}
#elif PLATFORM_UNIX
#include <stdlib.h>

namespace augs {
	int shell(const std::string& s) {
		return system(s.c_str());
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
		const auto full_path = std::experimental::filesystem::system_complete(augs::path_type(on_file));

		auto command_with_path = augs::path_type("$VISUAL ");
		command_with_path += full_path;

		command = command_with_path;
#else
#error "Unsupported platform!"
#endif
		shell(command.c_str());
	}
}
