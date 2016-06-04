#pragma once
#include <Windows.h>
#include "../math/rects.h"
#include "event.h"
#include "../misc/timer.h"
#include <functional>

#include "../graphics/renderer.h"
#include "colored_print.h"

namespace augs {
	namespace window {
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

			int minw, minh, maxw, maxh, cminw, cminh, cmaxw, cmaxh;
			int bpp = 0, style, exstyle, menu = 0, vsyn;
			
			std::wstring name;
			bool active = false, transparent = false, doublebuf;
			
			void _poll(event::message&, WPARAM, LPARAM);
			
			timer triple_timer;
			bool doubled = false;
		public:
			static glwindow* get_current();

			renderer glrenderer;

			bool raw_mouse_input = true;

			enum mode {
				MINIMIZE = SW_MINIMIZE,
				MAXIMIZE = SW_MAXIMIZE,
				RESTORE = SW_RESTORE,
				SHOW = SW_SHOW,
				HIDE = SW_HIDE
			} show;

			enum flag {
				CAPTION = WS_CAPTION,
				MENU = CAPTION | WS_SYSMENU,
				RESIZABLE = CAPTION | WS_THICKFRAME,
				MAXIMIZE_BOX = MENU | WS_MAXIMIZEBOX,
				MINIMIZE_BOX = MENU | WS_MINIMIZEBOX,
				ALL_WINDOW_ELEMENTS = CAPTION | MENU | MINIMIZE_BOX | MAXIMIZE_BOX
			};

			event::state events;

			/* user settings */
			std::function<void (glwindow&)> resize; /* resize function */
			unsigned triple_click_delay; /* maximum delay time for the next click (after doubleclick) to be considered tripleclick (in milliseconds) */
			
			glwindow();
			glwindow(const glwindow&) = delete;
			glwindow(glwindow&&) = delete;
			glwindow& operator=(const glwindow&) = delete;
			~glwindow();

			/*
			NEVER EVER PASS ~RESIZABLE!
			if you do so, adjustwindowrectex
			*/
			int create(rects::xywh<int> client_rectangle, int _menu = ALL_WINDOW_ELEMENTS, std::wstring name = L"Window", int doublebuffer = 1, int bitsperpixel = 24);
			    bool swap_buffers(), 
				focus_keyboard(), 
				current(),
				vsync(int);

			void initial_gl_calls();

			bool poll_event(event::message& out);
			std::vector<event::state> poll_events();

			void set_minimum_size(rects::wh<int> = rects::wh<int>()),
			     set_maximum_size(rects::wh<int> = rects::wh<int>());
			bool set_window_rect(const rects::xywh<int>&),
			     set_adjusted_rect(const rects::xywh<int>&),
			     set_show(mode);
			int  set_caption(const wchar_t*);

			rects::wh<int> 
				get_minimum_size() const, 
				get_maximum_size() const,
				get_screen_rect() const;
			rects::xywh<int>
				get_window_rect() const,
				get_adjusted_rect() const;

			int get_vsync() const;
			bool is_menu() const, is_transparent() const, is_active() const, is_doublebuffered() const;
			
			void adjust(rects::xywh<int>&);

			HWND get_hwnd() const;

			void destroy();
		};
	}
}