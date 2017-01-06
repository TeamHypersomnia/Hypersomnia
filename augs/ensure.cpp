#include "augs/ensure.h"
#include "augs/window_framework/platform_utils.h"

void cleanup_proc() {
	augs::window::disable_cursor_clipping();
	
	global_log::save_complete_log("logs/ensure_failed_debug_log.txt");

#ifdef PLATFORM_WINDOWS
  __debugbreak();
#endif
}
