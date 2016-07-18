#include "augs/ensure.h"
#include "augs/window_framework/platform_utils.h"

void cleanup_proc() {
	augs::window::disable_cursor_clipping();
	
#ifdef PLATFORM_WINDOWS
  __debugbreak();
#endif
}
