#pragma once
#if PLATFORM_WINDOWS
#include <Windows.h>
#undef min
#undef max
#endif
#include "augs/math/rects.h"
#include "event.h"
#include "augs/misc/timer.h"

#include "augs/graphics/renderer.h"
#include "colored_print.h"
#include "augs/audio/audio_manager.h"
#include "augs/templates/settable_as_current_mixin.h"

namespace augs {
	namespace window {
#ifdef PLATFORM_WINDOWS
    extern LRESULT CALLBACK wndproc(HWND, UINT, WPARAM, LPARAM);

		class glwindow : public augs::settable_as_current_mixin<glwindow> {
			friend int WINAPI ::WinMain (HINSTANCE, HINSTANCE, LPSTR, int);
			friend LRESULT CALLBACK wndproc(HWND, UINT, WPARAM, LPARAM);

			static glwindow* context;
			static bool window_class_registered;

			HWND hwnd = nullptr;
			HDC hdc = nullptr;
			HGLRC hglrc = nullptr;
			MSG wmsg;

			event::change latest_change;
			vec2i last_mouse_pos;

			vec2i min_window_size;
			vec2i max_window_size;

			int style = 0xdeadbeef;
			int exstyle = 0xdeadbeef;
			
			bool active = false;
			
			timer triple_timer;
			bool doubled = false;

			bool raw_mouse_input = true;
			unsigned triple_click_delay = 0xdeadbeef; /* maximum delay time for the next click (after doubleclick) to be considered tripleclick (in milliseconds) */
			
			void _poll(UINT&, WPARAM, LPARAM);
			bool poll_event(UINT& out);

			friend class augs::settable_as_current_mixin<glwindow>;
			void set_as_current_impl();
		public:
			glwindow();
			~glwindow();

			void create(
				const xywhi client_rectangle, 
				const bool enable_window_border = false, 
				const std::string name = "Window", 
				const int doublebuffer = 1, 
				const int bitsperpixel = 24
			);

			void set_window_name(const std::string& name);
			void set_window_border_enabled(bool);
			
			bool swap_buffers();

			void show();

			void destroy();

			std::vector<event::change> poll_events(const bool should_clip_cursor = true);

			void set_window_rect(const xywhi);
			vec2i get_screen_size() const;
			xywhi get_window_rect() const;

			bool is_active() const;
			
			glwindow(const glwindow&) = delete;
			glwindow(glwindow&&) = delete;
			glwindow& operator=(const glwindow&) = delete;
		};
#elif PLATFORM_LINUX
		// extern LRESULT CALLBACK wndproc(HWND, UINT, WPARAM, LPARAM);

		class glwindow {
			// friend int WINAPI ::WinMain (HINSTANCE, HINSTANCE, LPSTR, int);
			// friend LRESULT CALLBACK wndproc(HWND, UINT, WPARAM, LPARAM);

			// HWND hwnd = nullptr;
			// HDC hdc = nullptr;
			// HGLRC hglrc = nullptr;
			// MSG wmsg;
			// RECT srect;

			event::state events;

			int minw, minh, maxw, maxh, cminw, cminh, cmaxw, cmaxh;
			int bpp = 0, style, exstyle, menu = 0, vsyn;
			
			std::string name;
			bool active = false, transparent = false, doublebuf;
			
			
			timer triple_timer;
			bool doubled = false;

			bool raw_mouse_input = true;
			unsigned triple_click_delay; /* maximum delay time for the next click (after doubleclick) to be considered tripleclick (in milliseconds) */
			
			// void _poll(event::message&, WPARAM, LPARAM);
			bool poll_event(event::message& out);
		public:
			renderer gl;
			
			glwindow();
			~glwindow();

			int create(xywhi client_rectangle, int enable_window_border = 0, std::string name = "Window", int doublebuffer = 1, int bitsperpixel = 24);
			
			bool swap_buffers();

			void destroy();

			std::vector<event::state> poll_events();

			bool set_window_rect(const xywhi&);
			vec2i get_screen_size() const;
			xywhi get_window_rect() const;

			bool is_active() const;
			
			glwindow(const glwindow&) = delete;
			glwindow(glwindow&&) = delete;
			glwindow& operator=(const glwindow&) = delete;
		};
#endif
	}
}
