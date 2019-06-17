#include <iostream>
#include <clocale>

#include "augs/log.h"
#include "augs/log_path_getters.h"
#include "augs/filesystem/file.h"
#include "augs/window_framework/shell.h"

#include "cmd_line_params.h"
#include "build_info.h"

#if PLATFORM_WINDOWS
#include <Windows.h>
#undef MIN
#undef MAX
#endif

int work(const int argc, const char* const * const argv);

#if PLATFORM_WINDOWS
#if BUILD_IN_CONSOLE_MODE
int main(const int argc, const char* const * const argv) {
#else

HINSTANCE g_myhinst;

int __stdcall WinMain(HINSTANCE myhinst, HINSTANCE, char*, int) {
	g_myhinst = myhinst;
	const auto argc = __argc;
	const auto argv = __argv;
#endif
#elif PLATFORM_UNIX
int main(const int argc, const char* const * const argv) {
#else
#error "Unsupported platform!"
#endif
	std::setlocale(LC_NUMERIC, "C");

	const auto params = cmd_line_params(argc, argv);

	::is_dedicated_server = params.start_dedicated_server;

	if (params.help_only) {
		std::cout << get_help_section() << std::endl;
		
		return EXIT_SUCCESS;
	}

	const auto exit_code = work(argc, argv);

	{
		const auto logs = program_log::get_current().get_complete(); 

		switch (exit_code) {
			case EXIT_SUCCESS: 
				augs::save_as_text(get_exit_success_path(), logs); 
				break;
			case EXIT_FAILURE: {
				const auto failure_log_path = get_exit_failure_path();

				augs::save_as_text(failure_log_path, logs);
				augs::open_text_editor(failure_log_path);

				break;
			}

			default: 
				break;
		}
	}

	return exit_code;
}

