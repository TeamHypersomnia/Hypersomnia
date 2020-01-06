#pragma once
#include <future>
#include <mutex>

#include "augs/templates/thread_templates.h"
#include "augs/window_framework/exec.h"
#include "augs/misc/timing/timer.h"

#if PLATFORM_WINDOWS
#include <Windows.h>
#endif
#undef min
#undef max

namespace augs {
	std::wstring widen(const std::string& s);
}

struct archive_extractor {
	augs::timer extraction_timer;

#if PLATFORM_UNIX
	std::shared_ptr<FILE> pipe;

	void open_pipe(const std::string& resolved_command) {
		pipe = std::shared_ptr<FILE>(popen(resolved_command.c_str(), "r"), pclose);

		if (!pipe) {
			throw std::runtime_error("popen() failed!");
		}
	}
	
	std::optional<char> get_next_character() {
		if (!std::feof(pipe.get())) {
			if (const auto result = std::fgetc(pipe.get()); result != EOF) {
				return static_cast<char>(result);
			}
		}

		return std::nullopt;
	}

#elif PLATFORM_WINDOWS
	HANDLE g_hChildStd_OUT_Rd = NULL;
	HANDLE g_hChildStd_OUT_Wr = NULL;
	
	void ErrorExit(std::string content) {
		LOG("Error: %x", content);
		throw std::runtime_error(content);
	}

	void CreateChildProcess(const std::string& resolved_command) {
		STARTUPINFO si;
		PROCESS_INFORMATION pi;

		ZeroMemory( &si, sizeof(si) );
		si.cb = sizeof(si);
		
		si.hStdOutput = g_hChildStd_OUT_Wr;
		si.dwFlags |= STARTF_USESTDHANDLES;

		ZeroMemory( &pi, sizeof(pi) );

		const auto cmd = resolved_command;

		const auto cmd_wide = augs::widen(cmd);

		std::vector<wchar_t> cmd_line;
		cmd_line.resize(cmd_wide.size() + 1);
		std::memcpy(cmd_line.data(), cmd_wide.c_str(), sizeof(wchar_t) * (cmd_wide.size() + 1));

		// Start the child process. 
		if (!CreateProcess( 
			NULL,   // No module name (use command line)
			cmd_line.data(),        // Command line
			NULL,           // Process handle not inheritable
			NULL,           // Thread handle not inheritable
			TRUE,          
			CREATE_NO_WINDOW,              // No creation flags
			NULL,           // Use parent's environment block
			NULL,           // Use parent's starting directory 
			&si,            // Pointer to STARTUPINFO structure
			&pi            // Pointer to PROCESS_INFORMATION structure
		)) {
			ErrorExit("CreateProcess");
		}
		else{ 
			extraction_timer.reset();
			LOG("Closing unnecessary handles.");

			CloseHandle(pi.hProcess);
			CloseHandle(pi.hThread);
			CloseHandle(g_hChildStd_OUT_Wr);
		}
	}
	void open_pipe(const std::string& resolved_command) {
		SECURITY_ATTRIBUTES saAttr; 

		saAttr.nLength = sizeof(SECURITY_ATTRIBUTES); 
		saAttr.bInheritHandle = TRUE; 
		saAttr.lpSecurityDescriptor = NULL; 

		LOG("Create a pipe for the child process's STDOUT. ");

		if ( ! CreatePipe(&g_hChildStd_OUT_Rd, &g_hChildStd_OUT_Wr, &saAttr, 0) ) 
		ErrorExit(("StdoutRd CreatePipe")); 

		LOG("Ensure the read handle to the pipe for STDOUT is not inherited.");

		if ( ! SetHandleInformation(g_hChildStd_OUT_Rd, HANDLE_FLAG_INHERIT, 0) )
		ErrorExit(("Stdout SetHandleInformation")); 

		LOG("Create the child process."); 

		CreateChildProcess(resolved_command);
	}
	
	std::optional<char> get_next_character() {
		constexpr std::size_t BUFSIZE = 1;

		DWORD dwRead; 
		CHAR chBuf[BUFSIZE]; 
		BOOL bSuccess = FALSE;

		bSuccess = ReadFile( g_hChildStd_OUT_Rd, chBuf, BUFSIZE, &dwRead, NULL);
		
		if (!bSuccess || dwRead == 0) {
			return std::nullopt;
		} 

		return chBuf[0]; 
	}
#endif

	void deinit_pipe() {
#if PLATFORM_WINDOWS
		CloseHandle(g_hChildStd_OUT_Rd);
#endif
	}

	std::string currently_processed;
	int read_percent;
	int percent_complete = 0;

	struct info {
		std::string processed = "";
		int percent = 0;
	};

	info current;

	std::future<void> completed_extraction;
	std::mutex mtx;

	auto lock_info() {
		return std::unique_lock<std::mutex>(mtx);
	}

	std::atomic<bool>& exit_requested;

	void worker() {
		auto deinit_pipe_guard = augs::scope_guard([this]() {
			deinit_pipe();
		});
	
		std::string line;
		bool read_till_backspace = false;

		while (const auto maybe_new_char = get_next_character()) {
			if (exit_requested.load()) {
				LOG("Extraction worker: requested cancellation.");
				return;
			}

			const auto new_char = *maybe_new_char;

			if (new_char == '%') {
				if (const auto number = get_trailing_number(line)) {
					line = std::to_string(*number) + "% ";
					read_till_backspace = true;
				}
			}
			else {
				constexpr char BACKSPACE_v = 8;
				constexpr char CARRIAGE_RETURN_v = 13;

				if (new_char == BACKSPACE_v || new_char == CARRIAGE_RETURN_v) {
					if (read_till_backspace) {
						int num_files = -1;
						int percent = -1;
						std::string processed;

						LOG("Processing: %x", line);

						if (typesafe_sscanf(line, "%x%  %x - %x", percent, num_files, processed)) {
							if (processed.size() > 0) {
								auto lk = lock_info();
								current.processed = processed;
								current.percent = percent;
							}
							else {
								auto lk = lock_info();
								current.percent = percent;
							}
						}
						LOG_NVPS(percent, num_files, processed);

						read_till_backspace = false;
					}

					line.clear();
				}
				else {
					line += new_char;
				}
			}
		}

		LOG("Extraction took: %x secs", extraction_timer.get<std::chrono::seconds>());
	}

	auto get_info() {
		info result;

		{
			auto lk = lock_info();
			result = current;
		}

		return result;
	}

	bool has_completed() {
		if (valid_and_is_ready(completed_extraction)) {
			completed_extraction.get();
			return true;
		}

		return false;
	}

	archive_extractor(
		const augs::path_type& archive_path,
		const augs::path_type& destination_path,
		std::atomic<bool>& exit_requested
	) : exit_requested(exit_requested) {
		const auto ext = archive_path.extension().string();

		const auto resolved_command = [&]() {
			const auto& source = archive_path;
			const auto& target = destination_path;

			return typesafe_sprintf("%x -o%x", source, target);
		}();

		completed_extraction = launch_async(
			[this, resolved_command]() {
				open_pipe(resolved_command);
				worker();
			}
		);
	}
};
