#include "augs/ensure.h"
#include "augs/window_framework/platform_utils.h"

void cleanup_proc() {
	augs::disable_cursor_clipping();
	
	global_log::save_complete_log("generated/logs/ensure_failed_debug_log.txt");

#ifdef PLATFORM_WINDOWS
	__debugbreak();
#endif
}
