#if PLATFORM_WINDOWS
/*
	A separate translation unit, so that windows.h macros don't leak into the rest of the window code.
*/

#include <windows.h>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include "augs/misc/timing/timer.h"
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

		/*
			Straight to the procedure of GLFW's window class, past whatever subclassed the window or hooks its messages
			(e.g. an overlay that blocks the game's input while it is open) - like GLFW, which passes the release
			straight to its key input.
		*/

		const auto glfw_window_procedure = reinterpret_cast<WNDPROC>(GetClassLongPtrW(handle, GCLP_WNDPROC));

		CallWindowProcW(glfw_window_procedure, handle, WM_KEYUP, static_cast<WPARAM>(k.virtual_key), key_up);
	}
}

/*
	The other thing glfwPollEvents does after its loop: it puts the hidden cursor of the disabled mode back at
	the window's center, so that a click lands in the window even if another program takes away GLFW's clip
	(ClipCursor is one setting for the whole desktop). Each such move is one more mouse message - the very thing
	that is slow on some machines. Instead, like SDL's relative mouse mode, the cursor is clipped to one pixel
	at the window's center, so it cannot move at all. The game then reads raw mouse motion, which does not depend
	on where the cursor is - without raw motion the cursor is left alone.

	Like SDL3, the clip is checked again after a pump that saw the cursor move, which a pinned cursor cannot,
	and also every 100 ms (SDL2 checks every 3 s). A pixel that GLFW does not release is released here.
	Like SDL, only the foreground window locks: a window can be active in its own thread while another program
	is in front.
*/

static void lock_disabled_cursor(GLFWwindow* const window, const bool cursor_moved) {
	static auto lock = RECT();
	static auto locked = false;
	static auto since_check = augs::timer();

	const auto handle = glfwGetWin32Window(window);

	const bool should_lock =
		glfwGetInputMode(window, GLFW_CURSOR) == GLFW_CURSOR_DISABLED
		&& glfwGetInputMode(window, GLFW_RAW_MOUSE_MOTION) == GLFW_TRUE
		&& GetForegroundWindow() == handle
		&& !IsIconic(handle)
	;

	if (!should_lock) {
		if (locked) {
			locked = false;

			auto current = RECT();

			if (GetClipCursor(&current) && EqualRect(&current, &lock)) {
				ClipCursor(nullptr);
			}
		}

		return;
	}

	const bool check_due = !locked || cursor_moved || since_check.get<std::chrono::milliseconds>() >= 100.0;

	if (!check_due) {
		return;
	}

	since_check.reset();

	auto client = RECT();

	if (!GetClientRect(handle, &client) || IsRectEmpty(&client)) {
		return;
	}

	auto center = POINT { (client.left + client.right) / 2, (client.top + client.bottom) / 2 };
	ClientToScreen(handle, &center);

	/*
		Over Remote Desktop the cursor wobbles by a pixel - SDL leaves it 2 pixels of room there.
	*/

	const auto remote_desktop_room = static_cast<LONG>(GetSystemMetrics(SM_REMOTESESSION) ? 2 : 0);

	lock = RECT { center.x - remote_desktop_room, center.y, center.x + 1 + remote_desktop_room, center.y + 1 };
	locked = true;

	auto current = RECT();

	if (GetClipCursor(&current) && EqualRect(&current, &lock)) {
		return;
	}

	ClipCursor(&lock);
}

namespace augs {
	void bounded_poll_events(GLFWwindow* const window, const double budget_ms) {
		LARGE_INTEGER frequency;
		LARGE_INTEGER start;

		QueryPerformanceFrequency(&frequency);
		QueryPerformanceCounter(&start);

		const auto budget_ticks = static_cast<LONGLONG>(budget_ms * static_cast<double>(frequency.QuadPart) / 1000.0);

		MSG msg;
		auto cursor_moved = false;

		while (PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE)) {
			if (msg.message == WM_QUIT) {
				/*
					Same as GLFW: treat it as a close request.
				*/

				glfwSetWindowShouldClose(window, GLFW_TRUE);
			}
			else {
				if (msg.message == WM_MOUSEMOVE) {
					cursor_moved = true;
				}

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
		::lock_disabled_cursor(window, cursor_moved);
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
