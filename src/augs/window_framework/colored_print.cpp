#include <cstdio>
#include "augs/window_framework/colored_print.h"

#if PLATFORM_WINDOWS
#include <Windows.h>

WORD GetColorAttribute(const int color) {
	switch (color) {
	case 0:    	return FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
	case 1:    	return FOREGROUND_RED;
	case 2: 	return FOREGROUND_GREEN;
	case 3: 	return FOREGROUND_RED | FOREGROUND_GREEN;
	default:	return 0;
	}
}

namespace augs {
	void colored_print(const console_color color, const char* const  text) {
		const HANDLE stdout_handle = GetStdHandle(STD_OUTPUT_HANDLE);

		// Gets the current text color.
		CONSOLE_SCREEN_BUFFER_INFO buffer_info;
		GetConsoleScreenBufferInfo(stdout_handle, &buffer_info);
		const WORD old_color_attrs = buffer_info.wAttributes;

		// We need to flush the stream buffers into the console before each
		// SetConsoleTextAttribute call lest it affect the text that is already
		// printed but has not yet reached the console.
		fflush(stdout);
		SetConsoleTextAttribute(stdout_handle,
			GetColorAttribute(color) | FOREGROUND_INTENSITY);

		fflush(stdout);
		printf("%s\n", text);
		// Restores the text color.
		SetConsoleTextAttribute(stdout_handle, old_color_attrs);
	}
}
#else
namespace augs {
	void colored_print(const console_color color, const char* const text) {
	
	}
}
#endif

