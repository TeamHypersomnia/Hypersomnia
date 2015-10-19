#pragma once
#include <Windows.h>
#include "../math/rects.h"
#include "event.h"
#include "../misc/timer.h"
#include <functional>

struct lua_State;

namespace augs {
	namespace config {
		struct input_file;
	}

	namespace window {
		extern LRESULT CALLBACK wndproc(HWND, UINT, WPARAM, LPARAM);

		class glwindow {
			friend int WINAPI ::WinMain (HINSTANCE, HINSTANCE, LPSTR, int);
			friend LRESULT CALLBACK wndproc(HWND, UINT, WPARAM, LPARAM);

			static glwindow* context;
			static glwindow* getcurrent();

			HWND hwnd;
			HDC hdc;
			HGLRC hglrc;
			MSG wmsg;
			RECT srect;

			int minw, minh, maxw, maxh, cminw, cminh, cmaxw, cmaxh;
			int bpp, style, exstyle, menu, vsyn;
			
			std::wstring name;
			bool active, transparent, doublebuf;
			
			void _poll(event::message&, WPARAM, LPARAM);
			
			misc::timer triple_timer;
			bool doubled;
		public:
			static void colored_print(int, const char* text);

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
			
			/*
			NEVER EVER PASS ~RESIZABLE!
			if you do so, adjustwindowrectex
			*/
			int create(rects::xywh<int> client_rectangle, int _menu = ALL_WINDOW_ELEMENTS, std::wstring name = L"Window", int doublebuffer = 1, int bitsperpixel = 24);
				/*
				example:
				string window_name Window
				int fullscreen 0
				int window_x 30
				int window_y 0
				int resolution_w 800
				int resolution_h 800
				int window_border 0
				int doublebuffer 1
				int bpp 24
				*/
				// create(lua_State*, rects::wh<int> force_minimum_resolution, int _menu = ALL_WINDOW_ELEMENTS),
			     bool swap_buffers(), 
				 focus_keyboard(), 
				 current(),
				 vsync(int);
#ifdef INCLUDE_DWM
			bool transparency(bool);
#endif
			bool poll_events(event::message& out);

			void set_minimum_size(rects::wh<int> = rects::wh<int>()),
			     set_maximum_size(rects::wh<int> = rects::wh<int>());
			bool set_window_rect(const rects::xywh<int>&),
			     set_adjusted_rect(const rects::xywh<int>&),
			     set_show(mode);
			int  set_caption(const wchar_t*);

			mode get_show() const;
			const wchar_t* get_caption() const;

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
			~glwindow();
		};


		extern bool set_display(int width, int height, int bpp);
		extern rects::xywh<int> get_display();
		extern int get_refresh_rate();
		extern void warp_cursor(int x, int y);
		extern void set_cursor_visible(int flag);

		extern void mbx(const wchar_t* title, const wchar_t* content);
		extern void imbx(int title, int content);
		extern void smbx(std::wstring title, std::wstring content);
		extern void copy_clipboard (std::wstring& from);
		extern void paste_clipboard(std::wstring& to);

	}
}