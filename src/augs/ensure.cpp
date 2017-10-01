#include <iostream>

#include "augs/ensure.h"
#include "augs/filesystem/file.h"
#include "augs/window_framework/platform_utils.h"
#include "augs/window_framework/shell.h"

void save_log_and_terminate() {
	augs::disable_cursor_clipping();
	
	const auto logs = program_log::get_current().get_complete();
	const auto failure_log_path = augs::path_type("generated/logs/ensure_failed_debug_log.txt");

	augs::create_text_file(failure_log_path, logs);

	{
		const auto s = failure_log_path.string();
		/* Open text editor */
		augs::shell(s.c_str());
	}

	std::cerr << logs;

#if IS_PRODUCTION_BUILD
	std::terminate();
#else
	#if PLATFORM_WINDOWS
	__debugbreak();
	#endif
#endif
}
