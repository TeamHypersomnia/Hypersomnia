#pragma once

struct GLFWwindow;

namespace augs {
	/*
		Windows only - a replacement for glfwPollEvents.

		glfwPollEvents dispatches messages until the queue is empty.
		While the mouse moves over the window, a new WM_MOUSEMOVE is ready every report (1 ms at 1000 Hz).
		If something makes each mouse message expensive (e.g. a mouse hook of another program
		that the system calls inside PeekMessage), that loop keeps chasing new moves
		and stalls for tens of milliseconds.

		This is the same loop, except it stops once the budget is used up.
		Mouse moves are coalesced by the system and raw input waits in the queue,
		so whatever is left is dispatched the next time and nothing is lost.

		After the loop it releases a Shift or Win key that Windows sent no key up for (both Shifts held, Win+V),
		like glfwPollEvents. Instead of re-centering the disabled cursor after every loop like glfwPollEvents
		(one more mouse message each time), it locks it to the window's center like SDL.
	*/

	void bounded_poll_events(GLFWwindow* window, double budget_ms);

	/*
		Windows only - a replacement for glfwWaitEvents, which ends with the unbounded loop of glfwPollEvents.

		Blocks until a message is waiting, also one already seen but left in the queue
		by bounded_poll_events running out of its budget. glfwPostEmptyEvent wakes it too.
	*/

	void wait_for_messages();
}
