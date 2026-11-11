#pragma once
#include <mutex>
#include <atomic>
#include <thread>
#include <chrono>
#include <utility>
#include <optional>
#include <exception>

#include "augs/log.h"
#include "augs/math/vec2.h"
#include "augs/misc/timing/timer.h"
#include "augs/misc/measurements.h"
#include "augs/misc/machine_entropy.h"
#include "augs/window_framework/event.h"
#include "augs/window_framework/window.h"
#include "augs/window_framework/crash_report.h"
#include "application/main/game_frame_buffer.h"

/*
	Rendering on a thread of its own, the window on the main thread.

	Only the thread that created the window can take its messages, and GLFW wants that to be the main thread.
	So the main thread only takes the window's messages and applies what the game wants of the window
	(settings, cursor), sleeping in between. The GL thread executes the renderer's commands, swaps the window's buffers
	and trades frame buffers with the logic. At every trade it takes whatever input came so far, never waiting for it -
	a slow message pump only delays the input, never the frames. Such a pump happens on Windows, where a mouse hook
	of another program can make every mouse message cost ~1 ms (GLFW #2684).

	Where the window system forbids it (window::supports_gl_thread - macOS, Wayland) and on the web,
	the main thread does both, one after the other.
*/

struct window_requests {
	augs::window_settings settings;
	bool clip_cursor = false;
	bool pause_cursor = false;
	std::optional<vec2i> cursor_pos;
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
	window_requests requests;
	unsigned requests_version = 0;

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
		The GL thread, at every trade with the logic.
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
		The GL thread, at every trade - copies and wakes the window thread only for a change.
	*/

	void post_requests(
		const augs::window_settings& settings,
		const bool clip_cursor,
		const bool pause_cursor,
		const std::optional<vec2i> cursor_pos
	) {
		{
			auto lock = std::scoped_lock(requests_lk);

			const bool changed =
				requests_version == 0
				|| cursor_pos.has_value()
				|| requests.clip_cursor != clip_cursor
				|| requests.pause_cursor != pause_cursor
				|| !(requests.settings == settings)
			;

			if (!changed) {
				return;
			}

			requests.settings = settings;
			requests.clip_cursor = clip_cursor;
			requests.pause_cursor = pause_cursor;

			if (cursor_pos.has_value()) {
				requests.cursor_pos = cursor_pos;
			}

			++requests_version;
		}

		window.post_empty_event();
	}

	/*
		The window thread. Copies the requests only when newer than taken_version, and returns their version -
		0 until the GL thread first posts, so the window keeps its settings meanwhile.
	*/

	unsigned take_requests(window_requests& into, const unsigned taken_version) {
		auto lock = std::scoped_lock(requests_lk);

		if (requests_version != taken_version) {
			into.settings = requests.settings;
			into.clip_cursor = requests.clip_cursor;
			into.pause_cursor = requests.pause_cursor;
			into.cursor_pos = std::exchange(requests.cursor_pos, std::nullopt);
		}

		return requests_version;
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

	auto applied_version = 0u;
	auto applied_when_active = false;
	auto since_applied = augs::timer();

	auto pump = [&]() {
		const auto pump_timer = augs::timer();

		new_input.clear();
		window.collect_entropy(new_input);

		bridge.push_input(new_input, window.get_screen_size(), pump_timer.get<std::chrono::seconds>());
	};

	for (;;) {
		pump();

		/*
			Applied again on a new focus, and now and then in case a cursor grab failed.
		*/

		const auto version = bridge.take_requests(requests, applied_version);

		const bool should_apply =
			version != 0 && (
				version != applied_version
				|| window.is_active() != applied_when_active
				|| since_applied.get<std::chrono::milliseconds>() >= 100.0
			)
		;

		if (should_apply) {
			::apply_window_requests(window, requests);

			/*
				A warp happens once - not again when the same requests are applied later.
			*/

			requests.cursor_pos = std::nullopt;

			applied_version = version;
			applied_when_active = window.is_active();
			since_applied.reset();

			/*
				Whatever the window answered with (e.g. its new size) - before sleeping, as nothing might wake us for it.
			*/

			pump();
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

	auto gl_thread = [&]() {
		try {
			return std::thread([&window, &bridge, &gl_frame]() {
				augs::keep_stack_for_crash_report();

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
		}
		catch (...) {
			window.set_as_current();
			throw;
		}
	}();

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

/*
	frame: one frame of rendering, false once the game quits.

	False if either loop failed. The error is logged here - whatever work() doesn't catch would end the game
	without a saved log - and the logic is let out of the trade it waits for, or the exit would hang.
*/

template <class F>
bool run_main_loop(
	augs::window& window,
	window_thread_bridge& bridge,
	game_frame_buffer_swapper& swapper,
	const bool window_thread,
	F&& frame
) {
	auto failed = [&swapper](const char* const what) {
		LOG("The main loop failed: %x", what);
		swapper.abort();

		return false;
	};

	try {
		if (window_thread) {
			::run_with_gl_thread(window, bridge, std::forward<F>(frame));
		}
		else {
			while (frame()) {}
		}
	}
	catch (const std::exception& err) {
		return failed(err.what());
	}
	catch (...) {
		return failed("an exception of an unknown type");
	}

	return true;
}

/*
	In the synced op: the logic waits, and the buffer of its next frame is the write buffer by now.

	With the window on its own thread, the logic gets the input that came until this moment,
	and the window thread gets what the logic wants of the window.
	Otherwise the input was collected before the trade, and the window is changed right away.
*/

inline void sync_window_with_logic(
	augs::window& window,
	window_thread_bridge& bridge,
	const bool window_thread,
	const game_frame_buffer& finished,
	game_frame_buffer& next,
	augs::time_measurements& pump_time
) {
	if (window_thread) {
		next.screen_size = bridge.take_input(next.new_window_entropy, pump_time);

		bridge.post_requests(
			finished.new_settings,
			finished.should_clip_cursor,
			finished.should_pause_cursor,
			window.take_requested_cursor_pos()
		);
	}
	else {
		auto requests = window_requests();

		requests.settings = finished.new_settings;
		requests.clip_cursor = finished.should_clip_cursor;
		requests.pause_cursor = finished.should_pause_cursor;
		requests.cursor_pos = window.take_requested_cursor_pos();

		::apply_window_requests(window, requests);
	}
}
