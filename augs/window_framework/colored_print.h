#pragma once
enum console_color {
	RED = 1,
	GREEN = 2,
	YELLOW = 3
};

namespace augs {
	void colored_print(console_color, const char* text);
}
