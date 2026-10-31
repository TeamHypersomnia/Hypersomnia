#pragma once
#include <mutex>
#include <atomic>
#include <thread>
#include <utility>
#include <optional>
#include <exception>

#include "augs/math/vec2.h"
#include "augs/misc/timing/timer.h"
#include "augs/misc/measurements.h"
#include "augs/misc/machine_entropy.h"
#include "augs/window_framework/event.h"
#include "augs/window_framework/window.h"

/*
	Rendering on a thread of its own, the window on the main thread.

	Only the thread that created the window can take its messages, and GLFW wants that to be the main thread.
	When the main thread executed GL too, a slow message pump stalled whole frames - the logic waits for the buffer
	swap as well. On Windows a mouse hook of another program can make every mouse message cost ~1 ms (GLFW #2684).

	Now the main thread only takes the window's messages and applies what the game wants of the window
	(cursor, settings), sleeping in between. The GL thread executes the renderer's commands, swaps the window's buffers
	and trades frame buffers with the logic. It never waits for the window: at every trade it takes whatever input came so far.

	The web build keeps all of it on its one main loop.
*/

struct window_requests {
	augs::window_settings settings;
	bool clip_cursor = false;
	bool pause_cursor = false;
	std::optional<vec2i> cursor_pos;

	bool operator==(const window_requests&) const = default;
};

inline void apply_window_requests(augs::window& window, const window_requests& requests) {
	if (requests.cursor_pos.has_value()) {
		window.set_cursor_pos(*requests.cursor_pos);
	}

	window.apply(requests.settings);

	const bool clip = window.is_active() && requests.clip_cursor;

	window.set_cursor_clipping(clip);
	window.set_cursor_visible(!clip);
	window.set_mouse_pos_paused(requests.pause_cursor);
}

class window_thread_bridge {
	augs::window& window;

	std::mutex input_lk;
	augs::local_entropy input;
	vec2i screen_size;
	double pump_secs = 0.0;

	std::mutex requests_lk;
	std::optional<window_requests> requests;
	std::optional<vec2i> cursor_pos;

	std::atomic<bool> gl_thread_finished = false;
	std::exception_ptr gl_thread_error;

public:
	window_thread_bridge(augs::window& window) : window(window), screen_size(window.get_screen_size()) {}

	/*
		The window thread, after every pump.
	*/

	void push_input(const augs::local_entropy& new_input, const vec2i new_screen_size, const double secs) {
		auto lock = std::scoped_lock(input_lk);

		input.insert(input.end(), new_input.begin(), new_input.end());
		screen_size = new_screen_size;
		pump_secs += secs;
	}

	/*
		The GL thread, before every trade with the logic.
		The pump's time is measured per frame: summed over all the pumps since the last frame.
	*/

	vec2i take_input(augs::local_entropy& into, augs::time_measurements& pump_time) {
		auto lock = std::scoped_lock(input_lk);

		into.clear();
		std::swap(into, input);
		pump_time.measure(std::exchange(pump_secs, 0.0));

		return screen_size;
	}

	/*
		The GL thread, in the synced op - the logic waits meanwhile, so what it requested can be read.
		Wakes the window thread only for a change.
	*/

	void post_requests(window_requests new_requests) {
		bool wake_window_thread = false;

		{
			auto lock = std::scoped_lock(requests_lk);

			if (new_requests.cursor_pos.has_value()) {
				cursor_pos = std::exchange(new_requests.cursor_pos, std::nullopt);
				wake_window_thread = true;
			}

			if (!requests.has_value() || !(*requests == new_requests)) {
				requests = std::move(new_requests);
				wake_window_thread = true;
			}
		}

		if (wake_window_thread) {
			window.post_empty_event();
		}
	}

	/*
		The window thread. False until the GL thread first posts - the window keeps its settings meanwhile.
	*/

	bool take_requests(window_requests& into) {
		auto lock = std::scoped_lock(requests_lk);

		if (!requests.has_value()) {
			return false;
		}

		into = *requests;
		into.cursor_pos = std::exchange(cursor_pos, std::nullopt);

		return true;
	}

	void finish_gl_thread(const std::exception_ptr error) {
		gl_thread_error = error;
		gl_thread_finished.store(true);
		window.post_empty_event();
	}

	bool is_gl_thread_finished() const {
		return gl_thread_finished.load();
	}

	void rethrow_gl_thread_error() const {
		if (gl_thread_error != nullptr) {
			std::rethrow_exception(gl_thread_error);
		}
	}
};

inline void run_window_loop(augs::window& window, window_thread_bridge& bridge) {
	auto new_input = augs::local_entropy();
	auto requests = window_requests();

	for (;;) {
		{
			const auto pump_timer = augs::timer();

			new_input.clear();
			window.collect_entropy(new_input);

			bridge.push_input(new_input, window.get_screen_size(), pump_timer.get<std::chrono::seconds>());
		}

		if (bridge.take_requests(requests)) {
			::apply_window_requests(window, requests);
		}

		if (bridge.is_gl_thread_finished()) {
			return;
		}

		window.wait_events();
	}
}

/*
	gl_frame: one frame of the GL thread, false once the game quits.
	Returns when it did - rethrows what either thread threw.
*/

template <class F>
void run_with_gl_thread(augs::window& window, window_thread_bridge& bridge, F&& gl_frame) {
	/*
		A GL context is current on one thread at a time. It goes to the GL thread,
		and comes back after it for the destructors of the GL resources.
	*/

	augs::window::set_current_to_none();

	auto gl_thread = std::thread([&window, &bridge, &gl_frame]() {
		auto error = std::exception_ptr();

		try {
			window.set_as_current();

			while (gl_frame()) {}
		}
		catch (...) {
			error = std::current_exception();
		}

		augs::window::set_current_to_none();
		bridge.finish_gl_thread(error);
	});

	auto window_error = std::exception_ptr();

	try {
		::run_window_loop(window, bridge);
	}
	catch (...) {
		window_error = std::current_exception();

		/*
			The GL thread ends only once the game quits - tell the game the window was closed.
		*/

		auto close = augs::event::change();
		close.msg = augs::event::message::close;

		bridge.push_input({ close }, window.get_screen_size(), 0.0);
	}

	gl_thread.join();
	window.set_as_current();

	if (window_error != nullptr) {
		std::rethrow_exception(window_error);
	}

	bridge.rethrow_gl_thread_error();
}
