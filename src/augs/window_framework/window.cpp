#include "window.h"

#include <algorithm>

#include "augs/log.h"
#include "augs/templates/string_templates.h"

#include "platform_utils.h"

#ifdef PLATFORM_WINDOWS
#include "augs/window_framework/translate_windows_enums.h"

namespace augs {
	namespace window {
		LRESULT CALLBACK wndproc(HWND hwnd, UINT umsg, WPARAM wParam, LPARAM lParam) {
			if (umsg == WM_GETMINMAXINFO || umsg == WM_INPUT)
				return DefWindowProc(hwnd, umsg, wParam, lParam);

			static glwindow* wnd;
			wnd = (glwindow*)GetWindowLongPtr(hwnd, GWLP_USERDATA);

			if (umsg == WM_CREATE)
				AddClipboardFormatListener(hwnd);
			if (wnd) {
				if (umsg == WM_DESTROY) {
					RemoveClipboardFormatListener(hwnd);
				}
				else if (umsg == WM_SYSCOMMAND || umsg == WM_ACTIVATE || umsg == WM_INPUT) {
					(PostMessage(hwnd, umsg, wParam, lParam));
					return 0;
				}
				else if (umsg == WM_GETMINMAXINFO) {
					UINT in = umsg;
					wnd->_poll(in, wParam, lParam);
					return 0;
				}
				else if (umsg == WM_SIZE) {
					LRESULT res = DefWindowProc(hwnd, umsg, wParam, lParam);
					return res;
				}
			}
			return DefWindowProc(hwnd, umsg, wParam, lParam);
		}

		void glwindow::_poll(UINT& m, WPARAM wParam, LPARAM lParam) {
			using namespace event::keys;
			using namespace event;

			static POINTS p;
			static RECT* r;
			static long rw, rh;
			static MINMAXINFO* mi;
			static BYTE lpb[40];
			static UINT dwSize = 40;
			static RAWINPUT* raw;

			latest_change = change();
			auto& events = latest_change;

			switch (m) {
			case WM_CHAR:
				events.character.utf16 = wchar_t(wParam);
				if (events.character.utf16 > 255) {
					break;
				}
				//events.utf32 = unsigned(wParam);
				events.repeated = ((lParam & (1 << 30)) != 0);
				break;
			case WM_UNICHAR:
				//events.utf32 = unsigned(wParam);
				//events.repeated = ((lParam & (1 << 30)) != 0);
				break;

			case SC_CLOSE:
				break;

			case WM_SYSKEYUP:
				m = WM_KEYUP;
				events.repeated = ((lParam & (1 << 30)) != 0);
			case WM_SYSKEYDOWN:
				m = WM_KEYDOWN;
				events.repeated = ((lParam & (1 << 30)) != 0);
			case WM_KEYDOWN:
			{
				events.key.key = translate_key_with_lparam(lParam, wParam);
				events.repeated = ((lParam & (1 << 30)) != 0);
			}
			break;

			case WM_KEYUP:
			{
				events.key.key = translate_key_with_lparam(lParam, wParam);
			}

			break;

			case WM_MOUSEWHEEL:
				events.scroll.amount = GET_WHEEL_DELTA_WPARAM(wParam) / WHEEL_DELTA;
				break;
			case WM_LBUTTONDBLCLK:
			case WM_LBUTTONDOWN:
				events.key.key = key::LMOUSE;
				SetCapture(hwnd);

				if (m == WM_LBUTTONDOWN) {
					if (doubled && triple_timer.extract<std::chrono::milliseconds>() < triple_click_delay) {
						m = unsigned(message::ltripleclick);
						doubled = false;
					}
				}
				else {
					triple_timer.extract<std::chrono::microseconds>();
					doubled = true;
				}

				break;
			case WM_RBUTTONDBLCLK:
			case WM_RBUTTONDOWN:
				events.key.key = key::RMOUSE;
				break;
			case WM_MBUTTONDBLCLK:
			case WM_MBUTTONDOWN:
				events.key.key = key::MMOUSE;
				break;
			case WM_XBUTTONDBLCLK:
			case WM_XBUTTONDOWN:
				events.key.key = key::MOUSE4;
				break;
			case WM_XBUTTONUP:
				events.key.key = key::MOUSE4;
				break;
			case WM_LBUTTONUP:
				events.key.key = key::LMOUSE;
				if (GetCapture() == hwnd) ReleaseCapture(); break;
			case WM_RBUTTONUP:
				events.key.key = key::RMOUSE;
				break;
			case WM_MBUTTONUP:
				events.key.key = key::MMOUSE;
				break;
			case WM_MOUSEMOVE:
				if (!raw_mouse_input) {
					p = MAKEPOINTS(lParam);
					events.mouse.rel.x = p.x - last_mouse_pos.x;
					events.mouse.rel.y = p.y - last_mouse_pos.y;
					if (events.mouse.rel.x || events.mouse.rel.y) doubled = false;
					last_mouse_pos.x = p.x;
					last_mouse_pos.y = p.y;

					m = WM_MOUSEMOVE;
				}
				break;

			case WM_INPUT:
				if (raw_mouse_input) {
					GetRawInputData(reinterpret_cast<HRAWINPUT>(lParam), RID_INPUT,
						lpb, &dwSize, sizeof(RAWINPUTHEADER));

					raw = reinterpret_cast<RAWINPUT*>(lpb);

					if (raw->header.dwType == RIM_TYPEMOUSE) {
						events.mouse.rel.x = static_cast<short>(raw->data.mouse.lLastX);
						events.mouse.rel.y = static_cast<short>(raw->data.mouse.lLastY);

						m = WM_MOUSEMOVE;
					}
				}

				break;

			case WM_ACTIVATE:
				active = LOWORD(wParam) == WA_ACTIVE;
				break;

			case WM_GETMINMAXINFO:
				mi = (MINMAXINFO*)lParam;
				mi->ptMinTrackSize.x = min_window_size.x;
				mi->ptMinTrackSize.y = min_window_size.y;
				mi->ptMaxTrackSize.x = max_window_size.x;
				mi->ptMaxTrackSize.y = max_window_size.y;
				break;

			case WM_SYSCOMMAND:
				switch (wParam) {
				case SC_CLOSE: break;
				case SC_MINIMIZE: break;
				case SC_MAXIMIZE: break;
				default: DefWindowProc(hwnd, m, wParam, lParam); break;
				}
				m = wParam;
				break;

			default: DefWindowProc(hwnd, m, wParam, lParam); break;
			}

			events.msg = translate_enum(m);
		}

		glwindow* glwindow::context = nullptr;

		glwindow::glwindow() {
			triple_click_delay = GetDoubleClickTime();
		}

		void glwindow::create(
			const xywhi crect, 
			const int enable_window_border, 
			const std::string name,
			const int doublebuffer, 
			const int bpp
		) {
			if(!window_class_registered) {
				WNDCLASSEX wcl = { 0 };
				wcl.cbSize = sizeof(wcl);
				wcl.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
				wcl.lpfnWndProc = window::wndproc;
				wcl.cbClsExtra = 0;
				wcl.cbWndExtra = 0;
				wcl.hInstance = GetModuleHandle(NULL);
				wcl.hIcon = LoadIcon(0, IDI_APPLICATION);
				wcl.hCursor = LoadCursor(0, IDC_ARROW);
				wcl.hbrBackground = 0;
				wcl.lpszMenuName = 0;
				wcl.lpszClassName = L"AugmentedWindow";
				wcl.hIconSm = 0;

				ensure(RegisterClassEx(&wcl) != 0 && "class registering");
				window_class_registered = true;
			}

			enum flag {
				CAPTION = WS_CAPTION,
				MENU = CAPTION | WS_SYSMENU,
				ALL_WINDOW_ELEMENTS = CAPTION | MENU
			};

			const auto menu = enable_window_border ? ALL_WINDOW_ELEMENTS : 0; 

			style = menu ? (WS_OVERLAPPED | menu) | WS_CLIPSIBLINGS | WS_CLIPCHILDREN : WS_POPUP;
			exstyle = menu ? WS_EX_WINDOWEDGE : WS_EX_APPWINDOW;
			
			ensure((hwnd = CreateWindowEx(exstyle, L"AugmentedWindow", to_wstring(name).c_str(), style, 0, 0, 0, 0, 0, 0, GetModuleHandle(NULL), this)));

			set_window_rect(crect);

			PIXELFORMATDESCRIPTOR p;
			ZeroMemory(&p, sizeof(p));

			p.nSize = sizeof(p);
			p.nVersion = 1;
			p.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | (doublebuffer ? PFD_DOUBLEBUFFER : 0);
			p.iPixelType = PFD_TYPE_RGBA;
			p.cColorBits = bpp;
			p.cAlphaBits = 8;
			p.cDepthBits = 16;
			p.iLayerType = PFD_MAIN_PLANE;
			ensure(hdc = GetDC(hwnd));

			GLuint pf;
			ensure(pf = ChoosePixelFormat(hdc, &p));
			ensure(SetPixelFormat(hdc, pf, &p));

			ensure(hglrc = wglCreateContext(hdc));

				set_as_current();
			ShowWindow(hwnd, SW_SHOW);

			SetLastError(0);
			ensure(!(SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)this) == 0 && GetLastError() != 0));

#ifndef HID_USAGE_PAGE_GENERIC
#define HID_USAGE_PAGE_GENERIC         ((USHORT) 0x01)
#endif
#ifndef HID_USAGE_GENERIC_MOUSE
#define HID_USAGE_GENERIC_MOUSE        ((USHORT) 0x02)
#endif

			RAWINPUTDEVICE Rid[1];
			Rid[0].usUsagePage = HID_USAGE_PAGE_GENERIC;
			Rid[0].usUsage = HID_USAGE_GENERIC_MOUSE;
			Rid[0].dwFlags = RIDEV_INPUTSINK;
			Rid[0].hwndTarget = hwnd;
			RegisterRawInputDevices(Rid, 1, sizeof(Rid[0]));
		}

		bool glwindow::swap_buffers() {
			if (this != context) set_as_current();
			return SwapBuffers(hdc) != FALSE;
		}

		void glwindow::set_as_current_impl() {
			wglMakeCurrent(hdc, hglrc);
		}

		bool glwindow::poll_event(UINT& out) {
			if (PeekMessageW(&wmsg, hwnd, 0, 0, PM_REMOVE)) {
				//DispatchMessage(&wmsg); 
				out = wmsg.message;
				_poll(out, wmsg.wParam, wmsg.lParam);

				TranslateMessage(&wmsg);

				return true;
			}
			return false;
		}

		std::vector<event::change> glwindow::poll_events(const bool should_clip_cursor) {
			ensure(is_current());

			UINT msg;
			std::vector<event::change> output;

			while (poll_event(msg)) {
				auto& state = latest_change;

				if (!state.repeated) {
					output.push_back(state);
				}
			}

			if (should_clip_cursor && GetFocus() == hwnd) {
				enable_cursor_clipping(get_window_rect());
			}
			else {
				disable_cursor_clipping();
			}

			return output;
		}

		void glwindow::set_window_rect(const xywhi r) {
			static RECT wr = { 0 };
			ensure(SetRect(&wr, r.x, r.y, r.r(), r.b()));
			ensure(AdjustWindowRectEx(&wr, style, FALSE, exstyle));
			ensure(MoveWindow(hwnd, wr.left, wr.top, wr.right - wr.left, wr.bottom - wr.top, TRUE));
		}

		vec2i glwindow::get_screen_size() const {
			return get_window_rect().get_size();
		}

		xywhi glwindow::get_window_rect() const {
			static RECT r;
			GetClientRect(hwnd, &r);
			ClientToScreen(hwnd, (POINT*)&r);
			ClientToScreen(hwnd, (POINT*)&r + 1);
			return ltrbi(r.left, r.top, r.right, r.bottom);
		}

		bool glwindow::is_active() const {
			return active;
		}

		void glwindow::destroy() {
			if (hwnd) {
				if (is_current()) {
					wglMakeCurrent(NULL, NULL);
					wglDeleteContext(hglrc);
					set_current_to_none();
				}
				ReleaseDC(hwnd, hdc);
				DestroyWindow(hwnd);
				hwnd = nullptr;
				hdc = nullptr;
				hglrc = nullptr;
			}
		}

		glwindow::~glwindow() {
			destroy();
		}
	}
}

bool augs::window::glwindow::window_class_registered = false;

#elif PLATFORM_LINUX
#endif
