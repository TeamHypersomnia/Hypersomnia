#include "augs/window_framework/shell.h"
#include "augs/templates/string_templates.h"

#if PLATFORM_WINDOWS
#include <Windows.h>

namespace augs {
	void shell(const std::string& s) {
		const auto wide = to_wstring(s);

		ShellExecute(NULL, NULL, wide.c_str(), NULL, NULL, SW_SHOW);
	}
}
#elif PLATFORM_LINUX
#include <stdlib.h>

namespace augs {
	void shell(const std::string& s) {
		system(s.c_str());
	}
}
#else
namespace augs {
	void shell(const std::string& s) {

	}
}
#endif
