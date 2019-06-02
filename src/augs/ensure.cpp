#include <csignal>
#include <functional>

#include "augs/ensure.h"
#include "augs/filesystem/file.h"
#include "augs/window_framework/window.h"
#include "augs/window_framework/shell.h"
#include "augs/build_settings/compiler_defines.h"

extern std::function<void()> ensure_handler;
//void (*ensure_handler)() = nullptr;

FORCE_NOINLINE void save_log_and_terminate() {
	if (ensure_handler) {
		ensure_handler();
	}

	if (augs::window::current_exists()) {
		augs::window::get_current().disable_cursor_clipping();
	}
	
	const auto logs = program_log::get_current().get_complete();
	const auto failure_log_path = augs::path_type(get_path_in_log_files("ensure_failed_debug_log.txt"));

	augs::save_as_text(failure_log_path, logs);

	augs::open_text_editor(failure_log_path.string());

#if !IS_PRODUCTION_BUILD
	/* If we're not in production, trigger a debugger break so that we know the trace */
#if PLATFORM_WINDOWS
	__debugbreak();
#else
	/* On linux we can just script it to always break here */
#endif
#endif

	/* 
		Should force a core dump on Linux platforms, 
		and will also act as a breakpoint for gdb
	*/

	std::abort();	
}
