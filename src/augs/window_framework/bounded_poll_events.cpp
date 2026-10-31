#if PLATFORM_WINDOWS
/*
	A separate translation unit, so that windows.h macros don't leak into the rest of the window code.
*/

#include <windows.h>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include "augs/window_framework/bounded_poll_events.h"

namespace augs {
	void bounded_poll_events(GLFWwindow* const window, const double budget_ms) {
		LARGE_INTEGER frequency;
		LARGE_INTEGER start;

		QueryPerformanceFrequency(&frequency);
		QueryPerformanceCounter(&start);

		const auto budget_ticks = static_cast<LONGLONG>(budget_ms * static_cast<double>(frequency.QuadPart) / 1000.0);

		MSG msg;

		while (PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE)) {
			if (msg.message == WM_QUIT) {
				/*
					Same as GLFW: treat it as a close request.
				*/

				glfwSetWindowShouldClose(window, GLFW_TRUE);
			}
			else {
				TranslateMessage(&msg);
				DispatchMessageW(&msg);
			}

			LARGE_INTEGER now;
			QueryPerformanceCounter(&now);

			if (now.QuadPart - start.QuadPart >= budget_ticks) {
				break;
			}
		}
	}

	void wait_for_messages() {
		/*
			MWMO_INPUTAVAILABLE: without it, input that PeekMessage has already seen
			would not wake us up even though it is still in the queue.
		*/

		MsgWaitForMultipleObjectsEx(0, NULL, INFINITE, QS_ALLINPUT, MWMO_INPUTAVAILABLE);
	}
}
#endif
