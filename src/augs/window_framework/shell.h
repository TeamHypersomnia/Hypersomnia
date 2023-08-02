#pragma once
#include <string>

namespace augs {
	int shell(const std::string&);
	void open_url(const std::string&);
	void open_text_editor(const std::string& on_file);

	int restart_application(
		const int previous_argc,
		const char* const * const previous_argv,
		const std::string& executable,
		std::vector<std::string> arguments
	);

	std::string get_user_name();
}
