#pragma once
#include <mutex>
#include <atomic>
#include <condition_variable>

#include "game/enums/particle_layer.h"
#include "view/audiovisual_state/particle_triangle_buffers.h"

enum class renderer_type {
	// GEN INTROSPECTOR enum class renderer_type
	GENERAL,
	GAME_GUI,
	POST_GAME_GUI,
	DEBUG_DETAILS,

	COUNT
	// END GEN INTROSPECTOR
};

struct all_game_renderers {
	augs::enum_array<augs::renderer, renderer_type> all;

	void next_frame() {
		for (auto& a : all) {
			a.next_frame();
		}
	}

	std::size_t extract_num_total_triangles_drawn() {
		std::size_t total = 0;

		augs::for_each_enum_except_bounds([&](const renderer_type t) {
			if (t == renderer_type::DEBUG_DETAILS) {
				return;
			}

			total += all[t].extract_num_total_triangles_drawn();
		});

		return total;
	}
};

struct game_frame_buffer {
	augs::local_entropy new_window_entropy;
	augs::window_settings new_settings;
	swap_buffers_moment swap_when = swap_buffers_moment::AFTER_HELPING_LOGIC_THREAD;
	augs::maybe<int> max_fps = augs::maybe<int>::disabled(60);
	augs::max_fps_type max_fps_method = augs::max_fps_type::YIELD;

	bool should_clip_cursor = false;
	bool should_pause_cursor = false;
	bool should_quit = false;
	bool should_recheck_current_context = false;
	vec2i screen_size;

	all_game_renderers renderers;
	particle_triangle_buffers particle_buffers;
};

#define SWAP_BUFFERS_BY_POINTERS 1

class game_frame_buffer_swapper {
	std::atomic<bool> already_waiting = false;
	std::atomic<bool> swap_complete = false;

	std::mutex swapper_m;
	std::mutex waiter_m;

	std::condition_variable swapper_cv;
	std::condition_variable waiter_cv;

#if SWAP_BUFFERS_BY_POINTERS
	std::array<game_frame_buffer, 2> buffers;

	game_frame_buffer* read_buffer = nullptr;
	game_frame_buffer* write_buffer = nullptr;
#else
	game_frame_buffer read_buffer;
	game_frame_buffer write_buffer;
#endif

public:

	game_frame_buffer_swapper() {
#if SWAP_BUFFERS_BY_POINTERS
		read_buffer = &buffers[0];
		write_buffer = &buffers[1];
#endif
	}

#if SWAP_BUFFERS_BY_POINTERS
	game_frame_buffer& get_read_buffer() {
		return *read_buffer;
	}

	game_frame_buffer& get_write_buffer() {
		return *write_buffer;
	}
#else
	game_frame_buffer& get_read_buffer() {
		return read_buffer;
	}

	game_frame_buffer& get_write_buffer() {
		return write_buffer;
	}
#endif

	template <class T>
	bool try_swap_buffers(T&& synchronized_op) {
		{
			std::scoped_lock lk(swapper_m);

			if (!already_waiting.load()) {
				return false;
			}
		}

		swap_buffers(std::forward<T>(synchronized_op));
		return true;
	}

	template <class T>
	void swap_buffers(T&& synchronized_op) {
		{
			std::unique_lock<std::mutex> lk(swapper_m);
			swapper_cv.wait(lk, [this]{ return already_waiting.load(); });
		}


		{
			/* The other thread MUST be waiting while in this block */
			std::swap(read_buffer, write_buffer);
			already_waiting.store(false);

			synchronized_op();
		}

		notify_swap_completion();
	}

	void notify_swap_completion() {
		{
			std::scoped_lock lk(waiter_m);
			swap_complete.store(true);
		}

		waiter_cv.notify_all();
	}

	void wait_swap() {
		swap_complete.store(false);
		
		{
			std::scoped_lock lk(swapper_m);
			already_waiting.store(true);
		}

		swapper_cv.notify_all();

		{
			std::unique_lock<std::mutex> lk(waiter_m);
			waiter_cv.wait(lk, [this]{ return swap_complete.load(); });
		}
	}
};

