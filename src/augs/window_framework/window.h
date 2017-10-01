#pragma once
#if PLATFORM_WINDOWS
#include <Windows.h>
#undef min
#undef max
#endif

#include <optional>

#include "augs/math/rects.h"

#include "augs/templates/settable_as_current_mixin.h"

#include "augs/misc/timer.h"
#include "augs/misc/machine_entropy.h"

#include "augs/window_framework/colored_print.h"
#include "augs/window_framework/event.h"
#include "augs/window_framework/window_settings.h"

namespace augs {
	// extern LRESULT CALLBACK wndproc(HWND, UINT, WPARAM, LPARAM);
	class window : public augs::settable_as_current_mixin<window> {
#if PLATFORM_WINDOWS
		friend int WINAPI::WinMain(HINSTANCE, HINSTANCE, LPSTR, int);
		friend LRESULT CALLBACK wndproc(HWND, UINT, WPARAM, LPARAM);

		HWND hwnd = nullptr;
		HDC hdc = nullptr;
		HGLRC hglrc = nullptr;

		vec2i min_window_size;
		vec2i max_window_size;

		int style = 0xdeadbeef;
		int exstyle = 0xdeadbeef;

		bool active = false;
		bool double_click_occured = false;
		bool clear_window_inputs_once = true;

		bool mouse_position_frozen = false;
		vec2i last_mouse_pos;

		timer triple_click_timer;
		unsigned triple_click_delay = 0xdeadbeef; /* maximum delay time for the next click (after doubleclick) to be considered tripleclick (in milliseconds) */

		bool cursor_in_client_area = false;
		bool is_cursor_in_client_area() const;

		std::optional<event::change> handle_event(
			const UINT, 
			const WPARAM, 
			const LPARAM
		);

		friend class settable_as_current_base;
		bool set_as_current_impl();
		static void set_current_to_none_impl();
#elif PLATFORM_LINUX

#endif
		window_settings current_settings;

		void set_window_name(const std::string& name);
		void set_window_border_enabled(const bool);
		void set_window_rect(const xywhi);

		void destroy();
	public:
		window(const window_settings&);
		~window();

		window(window&&) = delete;
		window& operator=(window&&) = delete;

		window(const window&) = delete;
		window& operator=(const window&) = delete;

		bool swap_buffers();

		void show();
		void set_mouse_position_frozen(const bool);

		void apply(const window_settings&, const bool force = false);
		void sync_back_into(window_settings&);
		window_settings get_current_settings() const;

		local_entropy collect_entropy();
		void collect_entropy(local_entropy& into);

		vec2i get_screen_size() const;
		xywhi get_window_rect() const;

		bool is_active() const;
	};
}
