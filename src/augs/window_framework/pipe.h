#pragma once

#if PLATFORM_WINDOWS
#include <Windows.h>
#undef min
#undef max
#endif

namespace augs {
	std::wstring widen(const std::string& s);

	class pipe {
#if PLATFORM_UNIX
		std::shared_ptr<FILE> pipe_fd;

		void open_pipe(const std::string& resolved_command) {
			pipe_fd = std::shared_ptr<FILE>(popen(resolved_command.c_str(), "r"), pclose);

			if (!pipe_fd) {
				throw std::runtime_error("popen() failed!");
			}
		}
		
		void deinit_pipe() {

		}

	public:
		std::optional<char> get_next_character() {
			if (!std::feof(pipe_fd.get())) {
				if (const auto result = std::fgetc(pipe_fd.get()); result != EOF) {
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
		
		void deinit_pipe() {
			CloseHandle(g_hChildStd_OUT_Rd);
		}

	public:
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

		static std::string execute(const std::string& resolved_command) {
			auto p = pipe(resolved_command);

			std::string result;

			while (const auto maybe_new_char = p.get_next_character()) {
				result += *maybe_new_char;
			}

			return result;
		}

		pipe(const std::string& resolved_command) {
			open_pipe(resolved_command);
		}

		pipe(const pipe&) = delete;
		pipe(pipe&&) = delete;
		pipe& operator=(const pipe&) = delete;
		pipe& operator=(pipe&&) = delete;

		~pipe() {
			deinit_pipe();
		}
	};
}
