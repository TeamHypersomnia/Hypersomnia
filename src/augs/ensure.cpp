#include "augs/ensure.h"
#include "augs/window_framework/platform_utils.h"

void press_any_key() {
	LOG("Press any key...");
	getchar();
}

void press_any_key_and_exit() {
	press_any_key();
	std::terminate();
}

void cleanup_proc() {
	augs::disable_cursor_clipping();
	
	program_log::get_current().save_complete_to("generated/logs/ensure_failed_debug_log.txt");

#ifdef PLATFORM_WINDOWS
	__debugbreak();
#endif
}
