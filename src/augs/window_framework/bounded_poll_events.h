#pragma once

struct GLFWwindow;

namespace augs {
	/*
		Windows only - a replacement for glfwPollEvents.

		glfwPollEvents dispatches messages until the queue is empty.
		While the mouse moves over the window, a new WM_MOUSEMOVE is ready every report (1 ms at 1000 Hz).
		If something makes each mouse message expensive (e.g. a mouse hook of another program
		that the system calls inside PeekMessage), that loop keeps chasing new moves
		and the frame stalls for tens of milliseconds.

		This is the same loop, except it stops once the budget is used up.
		Mouse moves are coalesced by the system and raw input waits in the queue,
		so whatever is left is dispatched the next frame and nothing is lost.

		Skipped from glfwPollEvents after its loop:
		- re-centering the disabled cursor - not needed as raw mouse motion is always on Windows,
		- releasing a Shift or Win key that Windows sent no key up for (both Shifts held, Win+V).
	*/

	void bounded_poll_events(GLFWwindow* window, double budget_ms);
}
