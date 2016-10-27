#pragma once
#if PLATFORM_WINDOWS
#include <Windows.h>
#endif
#include "augs/math/rects.h"
#include "event.h"
#include "augs/misc/timer.h"

#include "augs/graphics/renderer.h"
#include "colored_print.h"

namespace augs {
	namespace window {
#ifdef PLATFORM_WINDOWS
    extern LRESULT CALLBACK wndproc(HWND, UINT, WPARAM, LPARAM);

		class glwindow {
			friend int WINAPI ::WinMain (HINSTANCE, HINSTANCE, LPSTR, int);
			friend LRESULT CALLBACK wndproc(HWND, UINT, WPARAM, LPARAM);

			static glwindow* context;

			HWND hwnd = nullptr;
			HDC hdc = nullptr;
			HGLRC hglrc = nullptr;
			MSG wmsg;
			RECT srect;

			event::state events;
			vec2i last_mouse_pos;

			int minw, minh, maxw, maxh, cminw, cminh, cmaxw, cmaxh;
			int bpp = 0, style, exstyle, menu = 0, vsyn;
			
			std::string name;
			bool active = false, transparent = false, doublebuf;
			
			
			timer triple_timer;
			bool doubled = false;

			bool raw_mouse_input = true;
			unsigned triple_click_delay; /* maximum delay time for the next click (after doubleclick) to be considered tripleclick (in milliseconds) */
			
			void _poll(UINT&, WPARAM, LPARAM);
			bool poll_event(UINT& out);
		public:
			renderer gl;
			
			glwindow();
			~glwindow();

			int create(rects::xywh<int> client_rectangle, int enable_window_border = 0, std::string name = "Window", int doublebuffer = 1, int bitsperpixel = 24);
			
			bool swap_buffers(), 
				set_as_current(),
				set_vsync(int);

			void destroy();

			std::vector<event::state> poll_events(const bool should_clip_cursor = true);

			bool set_window_rect(const rects::xywh<int>&);
			rects::wh<int> get_screen_rect() const;
			rects::xywh<int> get_window_rect() const;

			bool is_active() const;
			
			static glwindow* get_current();

			glwindow(const glwindow&) = delete;
			glwindow(glwindow&&) = delete;
			glwindow& operator=(const glwindow&) = delete;
		};
#elif PLATFORM_LINUX
		// extern LRESULT CALLBACK wndproc(HWND, UINT, WPARAM, LPARAM);

		class glwindow {
			// friend int WINAPI ::WinMain (HINSTANCE, HINSTANCE, LPSTR, int);
			// friend LRESULT CALLBACK wndproc(HWND, UINT, WPARAM, LPARAM);

			static glwindow* context;

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

			int create(rects::xywh<int> client_rectangle, int enable_window_border = 0, std::string name = "Window", int doublebuffer = 1, int bitsperpixel = 24);
			
			bool swap_buffers(), 
				set_as_current(),
				set_vsync(int);

			void destroy();

			std::vector<event::state> poll_events();

			bool set_window_rect(const rects::xywh<int>&);
			rects::wh<int> get_screen_rect() const;
			rects::xywh<int> get_window_rect() const;

			bool is_active() const;
			
			static glwindow* get_current();

			glwindow(const glwindow&) = delete;
			glwindow(glwindow&&) = delete;
			glwindow& operator=(const glwindow&) = delete;
		};
#endif
	}
}
