#include <iostream>
#include <clocale>

#include "augs/log.h"
#include "augs/log_path_getters.h"
#include "augs/filesystem/file.h"
#include "augs/window_framework/shell.h"

#include "application/main/verify_signature.h"

#include "cmd_line_params.h"
#include "build_info.h"

extern std::mutex log_mutex;

extern std::string live_log_path;
extern bool log_to_live_file;

#ifdef __APPLE__   
#include "CoreFoundation/CoreFoundation.h"
#include <unistd.h>
#include <libgen.h>

augs::path_type get_executable_path() {
	CFBundleRef mainBundle = CFBundleGetMainBundle();
	CFURLRef exeURL = CFBundleCopyExecutableURL(mainBundle);
	char path[PATH_MAX];
	if (!CFURLGetFileSystemRepresentation(exeURL, TRUE, (UInt8 *)path, PATH_MAX))
	{
		// error!
	}
	CFRelease(exeURL);

	return path;
}

#elif PLATFORM_UNIX
#include <unistd.h>
augs::path_type get_current_exe_path() {
	char dest[PATH_MAX];
	memset(dest,0,sizeof(dest)); // readlink does not null terminate!
	if (readlink("/proc/self/exe", dest, PATH_MAX) == -1) {

		} else {
			return dest;
		}

	return augs::path_type();
}
#endif

#if PLATFORM_WINDOWS
#include <Windows.h>
#undef MIN
#undef MAX
#endif

#include "augs/window_framework/create_process.h"
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

	if (params.version_line_only) {
		std::cout << hypersomnia_version().get_version_string() << std::endl;

		return EXIT_SUCCESS;
	}

#ifdef __APPLE__    
	const auto exe_path = get_executable_path();
#else
	const auto exe_path = params.exe_path;
#endif

	if (!params.keep_cwd) {
#ifdef __APPLE__    
		auto exe_dir = get_executable_path();
		exe_dir.replace_filename("");

		std::cout << "CHANGING CWD TO: " << exe_dir << std::endl;
		std::filesystem::current_path(exe_dir);
		std::cout << "CHANGED CWD TO: " << std::filesystem::current_path().string() << std::endl;

#elif PLATFORM_UNIX
		if (auto exe_path = get_current_exe_path(); !exe_path.empty()) {
			exe_path.replace_filename("");
			std::cout << "CHANGING CWD TO: " << exe_path.string() << std::endl;
			std::filesystem::current_path(exe_path);
			std::cout << "CHANGED CWD TO: " << exe_path.string() << std::endl;
		}
#endif
	}

	{
		std::unique_lock<std::mutex> lock(log_mutex);

		::current_app_type = params.type;

		if (!params.live_log_path.empty()) {
			::log_to_live_file = true;
			::live_log_path = params.live_log_path;
		}
		else {
			::live_log_path = ::get_path_in_log_files("live_debug.txt");
		}
	}

	if (params.help_only) {
		std::cout << get_help_section() << std::endl;
		
		return EXIT_SUCCESS;
	}

	if (params.version_only) {
		std::cout << get_version_section() << std::endl;

		return EXIT_SUCCESS;
	}

	if (!params.verified_archive.empty()) {
		const auto type = params.is_updater ? verified_object_type::UPDATER : verified_object_type::GAME;
		const auto result = ::verify_ssh_signature(params.verified_archive, params.verified_signature, type);

		if (result == ssh_verification_result::OK) {
			std::cout << "Signature Ok." << std::endl;
			return EXIT_SUCCESS;
		}
		else if (result == ssh_verification_result::NO_KEYGEN) {
			std::cout << "Failed to verify signature: couldn't open ssh-keygen." << std::endl;
			return EXIT_FAILURE;
		}
		else if (result == ssh_verification_result::FAILED_VERIFICATION) {
			std::cout << "Wrong signature." << std::endl;
			return EXIT_FAILURE;
		}
	}

	const auto completed_work_result = work(argc, argv);
	LOG_NVPS(completed_work_result);

	{
		auto save_success_logs = [&]() {
			const auto logs = program_log::get_current().get_complete(); 
			augs::save_as_text(get_exit_success_path(), logs); 
		};

		auto save_failure_logs = [&]() {
			const auto logs = program_log::get_current().get_complete(); 
			const auto failure_log_path = get_exit_failure_path();

			augs::save_as_text(failure_log_path, logs);
			augs::open_text_editor(failure_log_path);
			mark_as_controlled_crash();
		};

		switch (completed_work_result) {
			case work_result::REPORT_UPDATE_AVAILABLE: 
				LOG("Update available.");
				save_success_logs();
				return 1;

			case work_result::REPORT_UPDATE_UNAVAILABLE: 
				LOG("Update unavailable.");
				save_success_logs();
				return 0;

			case work_result::SUCCESS: 
				save_success_logs();
				return EXIT_SUCCESS;

			case work_result::FAILURE: {
				save_failure_logs();
				return EXIT_FAILURE;
			}

			case work_result::RELAUNCH_DEDICATED_SERVER: {
				LOG("main: Dedicated server requested relaunch.");
				save_success_logs();

				return augs::restart_application(argc, argv, exe_path.string(), { "--suppress-server-webhook" });
			}

			case work_result::RELAUNCH_UPGRADED: {
				LOG("main: Application requested relaunch due to a successful upgrade.");
				return augs::restart_application(argc, argv, exe_path.string(), { "--upgraded-successfully" });
			}

			default: 
				break;
		}
	}

	return EXIT_FAILURE;
}

