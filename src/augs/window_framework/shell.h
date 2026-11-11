#pragma once
#include <string>

#include "augs/filesystem/path_declaration.h"

namespace augs {
#if PLATFORM_WEB
	constexpr bool reveal_in_explorer_available_v = false;
#else
	constexpr bool reveal_in_explorer_available_v = true;
#endif

	inline std::string quoted_for_shell(const std::string& text) {
		auto quoted = std::string("'");

		for (const auto c : text) {
			if (c == '\'') {
				quoted += "'\\''";
			}
			else {
				quoted += c;
			}
		}

		quoted += "'";
		return quoted;
	}

	int shell(const std::string&);
	void open_url(const std::string&);
	void open_text_editor(const path_type& on_file);

	/*
		After this, open_text_editor does nothing - e.g. tests, where an editor opened on a failure log
		would also block the process (std::system waits for it).
	*/

	void disable_text_editor();

	int restart_application(
		const int previous_argc,
		const char* const * const previous_argv,
		const std::string& executable,
		std::vector<std::string> arguments
	);

	std::string get_user_name();
}
