#pragma once
#include <string>

namespace augs {
	int shell(const std::string&);
	void open_url(const std::string&);
	void open_text_editor(const std::string& on_file);

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
