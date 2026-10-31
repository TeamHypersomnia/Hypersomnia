#if PLATFORM_WINDOWS
/*
	A separate translation unit, so that windows.h macros don't leak into the rest of the window code.
*/

#include <windows.h>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include "augs/window_framework/bounded_poll_events.h"

/*
	What glfwPollEvents does after its loop. Windows sends no key up for a Shift released while the other one is held,
	nor for Win after Win+V. A key that is up by now but still down in GLFW is released through GLFW's own window
	procedure, so that its key state agrees - otherwise the next press would come as a repeat, which the game drops.
*/

static void release_keys_left_down(GLFWwindow* const window) {
	const auto handle = glfwGetWin32Window(window);

	if (GetActiveWindow() != handle) {
		return;
	}

	struct left_down_key {
		int virtual_key;
		int glfw_key;
	};

	const left_down_key keys[] = {
		{ VK_LSHIFT, GLFW_KEY_LEFT_SHIFT },
		{ VK_RSHIFT, GLFW_KEY_RIGHT_SHIFT },
		{ VK_LWIN, GLFW_KEY_LEFT_SUPER },
		{ VK_RWIN, GLFW_KEY_RIGHT_SUPER }
	};

	for (const auto& k : keys) {
		const bool down_in_windows = (GetKeyState(k.virtual_key) & 0x8000) != 0;

		if (down_in_windows || glfwGetKey(window, k.glfw_key) != GLFW_PRESS) {
			continue;
		}

		/*
			A key up's lParam: repeat count 1, the scancode with its extended bit, previously down, now going up.
		*/

		const auto scancode = static_cast<LPARAM>(glfwGetKeyScancode(k.glfw_key));

		const auto key_up =
			static_cast<LPARAM>(1)
			| ((scancode & 0xFF) << 16)
			| ((scancode & 0x100) << 16)
			| (static_cast<LPARAM>(1) << 30)
			| (static_cast<LPARAM>(1) << 31)
		;

		SendMessageW(handle, WM_KEYUP, static_cast<WPARAM>(k.virtual_key), key_up);
	}
}

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

		::release_keys_left_down(window);
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
