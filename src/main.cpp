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

#include "augs/window_framework/create_process.h"
#include "application/main/new_and_old_hypersomnia_path.h"
#include "work_result.h"

work_result work(const int argc, const char* const * const argv);

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
	/* 
		At least on Linux, 
		we need to call this in order to be able to write non-English characters. 
	*/

	std::setlocale(LC_ALL, "");
	std::setlocale(LC_NUMERIC, "C");

	const auto params = cmd_line_params(argc, argv);

	::is_dedicated_server = params.start_dedicated_server;

	if (params.help_only) {
		std::cout << get_help_section() << std::endl;
		
		return EXIT_SUCCESS;
	}

	if (params.version_only) {
		std::cout << get_version_section() << std::endl;

		return EXIT_SUCCESS;
	}

	const auto result = work(argc, argv);

	{
		const auto logs = program_log::get_current().get_complete(); 

		switch (result) {
			case work_result::SUCCESS: 
				augs::save_as_text(get_exit_success_path(), logs); 

				/* 
					Clean the remnants of the update if the new game version
					has at least once exited successfully.
				*/

				std::filesystem::remove_all(NEW_HYPERSOMNIA);

				try {
					std::filesystem::remove_all(OLD_HYPERSOMNIA);
				}
				catch (const augs::filesystem_error&) {

				}

				return EXIT_SUCCESS;

			case work_result::FAILURE: {
				const auto failure_log_path = get_exit_failure_path();

				augs::save_as_text(failure_log_path, logs);
				augs::open_text_editor(failure_log_path);

				return EXIT_FAILURE;
			}

			case work_result::RELAUNCH_UPGRADED: {
				augs::save_as_text(get_exit_success_path(), logs); 
				return augs::restart_application("--upgraded-successfuly");
			}

			default: 
				break;
		}
	}

	return EXIT_FAILURE;
}

