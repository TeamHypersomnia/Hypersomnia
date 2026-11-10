#pragma once
#include <string>

namespace augs {
	namespace x11_clipboard {
		void set_text(const std::string&);
		std::string get_text();
	}
}
