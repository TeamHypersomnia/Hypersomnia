#include <iostream>
#include <clocale>
#include <mutex>

#include "augs/log.h"
#include "augs/log_path_getters.h"
#include "augs/filesystem/file.h"
#include "augs/window_framework/shell.h"

#include "application/main/verify_signature.h"

#include "cmd_line_params.h"
#include "build_info.h"
#include "augs/window_framework/platform_utils.h"
#include "augs/filesystem/directory.h"
#include "augs/misc/date_time.h"
#include "all_paths.h"

augs::path_type CALLING_CWD;
augs::path_type APPDATA_DIR;
augs::path_type USER_DIR;

extern std::mutex log_mutex;

extern std::string live_log_path;
extern bool log_to_live_file;

#if PLATFORM_WINDOWS
#include <Windows.h>
#undef MIN
#undef MAX
#endif

#include "augs/window_framework/create_process.h"
#include "work_result.h"

work_result work(
	const cmd_line_params& parsed_params,
	bool log_directory_existed,
	int argc,
	const char* const * const argv
);

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

	/*
		For some reason, when launching through Steam, Polish characters didn't work.
		This is because std::setlocale(LC_ALL, "") results in LC_ALL being set to "C", 
		even though a non-Steam client will correctly set it to e.g. en_US.UTF-8.
	
		Setting LC_CTYPE to a locale-agnostic UTF-8 encoding seems to fix this.
	*/
#if PLATFORM_LINUX
	std::setlocale(LC_CTYPE, "C.UTF-8");
#endif

	auto params = cmd_line_params(argc, argv);

	if (params.version_line_only) {
		std::cout << hypersomnia_version().get_version_string() << std::endl;

		return EXIT_SUCCESS;
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

#ifdef __APPLE__    
	const auto exe_path = augs::get_executable_path();
#else
	const auto exe_path = params.exe_path;
#endif

	const bool is_appimage = !params.appimage_path.empty();

	::CALLING_CWD = augs::get_current_working_directory();

	if (params.calling_cwd.has_value()) {
		::CALLING_CWD = params.calling_cwd.value();
	}

#if PLATFORM_MACOS
	const bool force_keep_cwd = params.unit_tests_only;

	if (!force_keep_cwd) {
		if (auto exe_path = augs::get_executable_path(); !exe_path.empty()) {
			exe_path.replace_filename("");
			std::cout << "CHANGING CWD TO: " << exe_path.string() << std::endl;
			std::filesystem::current_path(exe_path);
			std::cout << "CHANGED CWD TO: " << exe_path.string() << std::endl;
		}
	}
#endif

	if (!params.appdata_dir.empty()) {
		if (params.appdata_dir.is_absolute()) {
			::APPDATA_DIR = params.appdata_dir;
		}
		else {
			::APPDATA_DIR = CALLING_CWD / params.appdata_dir;
		}
	}
	else {
		::APPDATA_DIR = augs::get_default_documents_dir();
	}

	const bool log_directory_existed = augs::exists(LOGS_DIR);

	/* 
		LOGS_DIR is always in APPDATA_DIR
		so it will create the documents dir as well.
	*/

	augs::create_directories(LOGS_DIR);

	{
		std::unique_lock<std::mutex> lock(log_mutex);

		::current_app_type = params.type;

		if (!params.live_log_path.empty()) {
			::log_to_live_file = true;
			::live_log_path = params.live_log_path;
		}
	}

	/*
		Now that the documents/logs folder exists,
		and we have set the potential live log path,
		we can start logging.
	*/

	LOG("Started at %x", augs::date_time().get_readable());

	if (is_appimage) {
		LOG("AppImage called from directory: \"%x\"", CALLING_CWD);
	}
	else {
		LOG("Executable called from directory: \"%x\"", CALLING_CWD);
	}

	LOG("Chosen working directory: \"%x\"", augs::get_current_working_directory());

	LOG("If the game crashes repeatedly, consider deleting the \"cache\" folder.\n");

	if (params.as_service) {
		LOG("Running the app as a service due to --as-service flag.");

		const auto launch_flags_path = LOGS_DIR / "launch.flags";

		try {
			const auto lines = augs::file_to_lines(launch_flags_path);

			if (lines.empty()) {
				LOG("service: launch.flags file was empty.");
			}
			else {
				LOG("service launch.flags has %x elements:\n%x", lines.size(), lines);
			}

			std::vector<const char*> argv;

			for (const auto& l : lines) {
				argv.push_back(l.c_str());
			}

			params.parse(argv.size(), argv.data(), 0);

			augs::remove_file(launch_flags_path);
		}
		catch (...) {
			LOG("service: no launch.flags file detected.");
		}
	}

	LOG("Complete command line:\n%x", params.complete_command_line);
	LOG("Parsed as:\n%x", params.parsed_as);

	if (is_appimage) {
		LOG("Running from an AppImage: %x", params.appimage_path);
	}

	if (::log_to_live_file) {
		LOG(std::string("Live log was enabled due to a flag: --live-log ") + params.live_log_path);
	}

	if (params.appdata_dir.empty()) {
		LOG("App data directory: \"%x\" (Default)", ::APPDATA_DIR);
	}
	else {
		LOG(
			"App data directory: \"%x\" (via --appdata-dir %x)", 
			::APPDATA_DIR,
			params.appdata_dir
		);
	}

	const auto completed_work_result = work(
		params,
		log_directory_existed,
		argc,
		argv
	);

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
			case work_result::STEAM_RESTART: 
				LOG("Game was started without Steam client. Restarting.");
				save_success_logs();
				return 1;

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

			case work_result::RELAUNCH_AND_UPDATE_DEDICATED_SERVER: {
				LOG("main: Dedicated server detected available updates.");
				save_success_logs();

				return augs::restart_application(argc, argv, exe_path.string(), { "--update-once-now --suppress-server-webhook" });
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

