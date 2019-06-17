#include "augs/window_framework/create_process.h"

#if PLATFORM_UNIX
#include <unistd.h>
#endif

#include "augs/window_framework/shell.h"
#include "augs/ensure.h"

#if PLATFORM_WINDOWS
#include <Windows.h>
#endif

namespace augs {
	bool spawn_detached_process(const std::string& executable, const std::string& arguments) {
#if PLATFORM_UNIX
		(void)executable;
		(void)arguments;

		const auto cmd = "nohup \"" + executable + "\" " + arguments + " &";
		augs::shell(cmd);

#if NDEBUG
		// TODO IMPLEMENT THIS IN A WAY THAT WORKS!!!
		ensure(false && "NOT IMPLEMENTED!!");
#endif
		return true;

#elif PLATFORM_WINDOWS
		STARTUPINFO si;
		PROCESS_INFORMATION pi;

		ZeroMemory( &si, sizeof(si) );
		si.cb = sizeof(si);
		ZeroMemory( &pi, sizeof(pi) );

		const auto cmd = "\"" + executable + "\" " + arguments;

		// Start the child process. 
		const auto result = CreateProcess( 
			NULL,   // No module name (use command line)
			cmd.c_str(),        // Command line
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
