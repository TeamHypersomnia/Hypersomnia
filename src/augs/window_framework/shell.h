#pragma once
#include <string>

namespace augs {
	int shell(const std::string&);
	void open_text_editor(const std::string& on_file);

	int restart_application(const std::string& arguments);
}
