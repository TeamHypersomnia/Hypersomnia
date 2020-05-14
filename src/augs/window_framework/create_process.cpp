#include "augs/window_framework/create_process.h"
#include <vector>

#if PLATFORM_UNIX
#include <unistd.h>
#endif

#include "augs/window_framework/shell.h"
#include "augs/ensure.h"

#if PLATFORM_WINDOWS
#include <Windows.h>
#endif

namespace augs {
#if PLATFORM_WINDOWS
	std::wstring widen(const std::string& s);
#endif

	bool spawn_detached_process(const std::string& executable, const std::string& arguments) {
#if PLATFORM_UNIX
		(void)executable;
		(void)arguments;

		const auto cmd = "( \"" + executable + "\" " + arguments + " & )";
		augs::shell(cmd);

		return true;

#elif PLATFORM_WINDOWS
		STARTUPINFO si;
		PROCESS_INFORMATION pi;

		ZeroMemory( &si, sizeof(si) );
		si.cb = sizeof(si);
		ZeroMemory( &pi, sizeof(pi) );

		const auto cmd = "\"" + executable + "\" " + arguments;

		const auto cmd_wide = widen(cmd);

		std::vector<wchar_t> cmd_line;
		cmd_line.resize(cmd_wide.size() + 1);
		std::memcpy(cmd_line.data(), cmd_wide.c_str(), sizeof(wchar_t) * (cmd_wide.size() + 1));

		// Start the child process. 
		const auto result = CreateProcess( 
			NULL,   // No module name (use command line)
			cmd_line.data(),        // Command line
			NULL,           // Process handle not inheritable
			NULL,           // Thread handle not inheritable
			FALSE,          // Set handle inheritance to FALSE
			0,              // No creation flags
			NULL,           // Use parent's environment block
			NULL,           // Use parent's starting directory 
			&si,            // Pointer to STARTUPINFO structure
			&pi            // Pointer to PROCESS_INFORMATION structure
		);

		return result;
#else
		(void)executable;
		(void)arguments;
		return false;
#endif
	}
}
