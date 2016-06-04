#include "window.h"

#include <GL/OpenGL.h>
#include <algorithm>

#include "log.h"
#include "templates.h"

#include "augs/error/augs_error.h"
#include "platform_utils.h"

namespace augs {
	extern HINSTANCE hinst;

	namespace window {
		LRESULT CALLBACK wndproc (HWND hwnd, UINT umsg, WPARAM wParam, LPARAM lParam) {
			static glwindow* wnd; 
			wnd = (glwindow*)GetWindowLongPtr(hwnd, GWLP_USERDATA);

			if(umsg == WM_CREATE)
				AddClipboardFormatListener(hwnd);
			if(wnd) {
				if(umsg == WM_DESTROY) {
					RemoveClipboardFormatListener(hwnd); 
				}
				else if(umsg == WM_SYSCOMMAND || umsg == WM_ACTIVATE || umsg == WM_INPUT) {
					err(PostMessage(hwnd, umsg, wParam, lParam));
					return 0;
				}
				else if(umsg == WM_GETMINMAXINFO) {
					event::message in = event::message(umsg);
					wnd->_poll(in, wParam, lParam);
					return 0;
				}
				else if(umsg == WM_SIZE) {
					LRESULT res = DefWindowProc(hwnd, umsg, wParam, lParam);
					return res;
				}
			} 
			return DefWindowProc(hwnd, umsg, wParam, lParam);
		}

		void glwindow::_poll(event::message& m, WPARAM wParam, LPARAM lParam) {
				using namespace event::keys;
				using namespace event;

				static POINTS p;
				static RECT* r;
				static long rw, rh;
				static MINMAXINFO* mi;
				static BYTE lpb[40];
				static UINT dwSize = 40;
				static RAWINPUT* raw;

				events.repeated = false;
				events.mouse.rel.set(0, 0);
				events.key_event = key_changed::NONE;

				switch (m) {
				case character:
					events.utf16 = wchar_t(wParam);
					if(events.utf16 > 255) {
						break;
					}
					events.utf32 = unsigned(wParam);
					break;
				case WM_UNICHAR:
					events.utf32 = unsigned(wParam);
					break;

				case event::close:
					break;

				case WM_SYSKEYUP:
					m = events.msg = keyup;
				case WM_SYSKEYDOWN:
					m = events.msg = keydown;
				case keydown:
					//if (!((lParam & (1 << 30)) != 0)) {
						// events.keys[wParam] = true;
						switch(wParam) {
						case CTRL: wParam = (lParam & 0x1000000) ? RCTRL : LCTRL; break;
						case SHIFT: wParam = (lParam & 0x1000000) ? RSHIFT : LSHIFT; break;
						case ALT: wParam = (lParam & 0x1000000) ? RALT : LALT; break;
						}
						events.keys[wParam] = true;
						events.key = key(wParam);
						events.key_event = event::PRESSED;
						events.repeated = ((lParam & (1 << 30)) != 0);
					//}
					break;								

				case keyup:							
					//events.keys[wParam] = false;
					switch (wParam) {
					case CTRL: wParam = (lParam & 0x1000000) ? RCTRL : LCTRL; break;
					case SHIFT: wParam = (lParam & 0x1000000) ? RSHIFT : LSHIFT; break;
					case ALT: wParam = (lParam & 0x1000000) ? RALT : LALT; break;
					}

					events.keys[wParam] = false;
					events.key = key(wParam);
					events.key_event = event::RELEASED;

					break;

				case wheel:
					events.mouse.scroll = GET_WHEEL_DELTA_WPARAM(wParam) / WHEEL_DELTA;
					break;
				case ldoubleclick:
				case ldown:
					events.key_event = event::PRESSED;
					events.key = LMOUSE;
					SetCapture(hwnd);
					events.mouse.state[0] = events.keys[LMOUSE] = true;

					if (m == ldown) {
						if (doubled && triple_timer.extract<std::chrono::milliseconds>() < triple_click_delay) {
							m = events.msg = ltripleclick;
							doubled = false;
						}
					}
					else {
						triple_timer.extract<std::chrono::microseconds>();
						doubled = true;
					}

					break;
				case rdoubleclick:
				case rdown:
					events.key_event = event::PRESSED;
					events.key = RMOUSE;
					events.mouse.state[1] = events.keys[RMOUSE] = true;  break;
				case mdoubleclick:
				case mdown:
					events.key_event = event::PRESSED;
					events.key = MMOUSE;
					events.mouse.state[2] = events.keys[MMOUSE] = true;  break;
				case WM_XBUTTONDBLCLK:
				case WM_XBUTTONDOWN:
					events.key_event = event::PRESSED;
					events.key = MOUSE4;
					events.keys[MOUSE4] = true;
					events.msg = keydown;
					break;
				case WM_XBUTTONUP:
					events.key_event = event::RELEASED;
					events.key = MOUSE4;
					events.keys[MOUSE4] = false;
					events.msg = keyup;
					break;
				case lup:				    
					events.key_event = event::RELEASED;
					events.key = LMOUSE;
					events.mouse.state[0] = events.keys[LMOUSE] = false; if(GetCapture() == hwnd) ReleaseCapture(); break;
				case rup:				    
					events.key_event = event::RELEASED;
					events.key = RMOUSE;
					events.mouse.state[1] = events.keys[RMOUSE] = false; break;
				case mup:				    
					events.key_event = event::RELEASED;
					events.key = MMOUSE;
					events.mouse.state[2] = events.keys[MMOUSE] = false; break;
				case mousemotion:
					if (!raw_mouse_input) {
						p = MAKEPOINTS(lParam);
						events.mouse.rel.x = p.x - events.mouse.pos.x;
						events.mouse.rel.y = p.y - events.mouse.pos.y;
						if (events.mouse.rel.x || events.mouse.rel.y) doubled = false;
						events.mouse.pos.x = p.x;
						events.mouse.pos.y = p.y;

						if (!events.mouse.state[0]) {
							events.mouse.ldrag.x = events.mouse.pos.x;
							events.mouse.ldrag.y = events.mouse.pos.y;
						}

						if (!events.mouse.state[1]) {
							events.mouse.rdrag.x = events.mouse.pos.x;
							events.mouse.rdrag.y = events.mouse.pos.y;
						}

						m = events.msg = mousemotion;
					}
					break;

				case WM_INPUT:
					if (raw_mouse_input) {
						GetRawInputData(reinterpret_cast<HRAWINPUT>(lParam), RID_INPUT,
							lpb, &dwSize, sizeof(RAWINPUTHEADER));

						raw = reinterpret_cast<RAWINPUT*>(lpb);

						if (raw->header.dwType == RIM_TYPEMOUSE) {
							events.mouse.rel.x = raw->data.mouse.lLastX;
							events.mouse.rel.y = raw->data.mouse.lLastY;

							m = events.msg = mousemotion;
						}
					}

					break;

				case event::activate:	
					active = LOWORD(wParam) == WA_ACTIVE;
					break;

				case WM_GETMINMAXINFO:
						mi = (MINMAXINFO*)lParam;
						mi->ptMinTrackSize.x = minw;
						mi->ptMinTrackSize.y = minh;
						mi->ptMaxTrackSize.x = maxw;
						mi->ptMaxTrackSize.y = maxh;
					break;

				case WM_SYSCOMMAND:
					switch(wParam) {
					case SC_CLOSE: break;
					case SC_MINIMIZE: break;
					case SC_MAXIMIZE: break;
					default: DefWindowProc(hwnd, events.msg, wParam, lParam); break;
					}
					m = events.msg = event::message(wParam);
					break;

				default: DefWindowProc(hwnd, events.msg, wParam, lParam); return;
				}
		}

		glwindow* glwindow::get_current() {
			return context;
		}

		glwindow* glwindow::context = nullptr;
		
		glwindow::glwindow() {
			for (int i = 0; i < 256; ++i)
				events.keys[i] = false;
			
			triple_click_delay = GetDoubleClickTime();
		}

		int glwindow::create(rects::xywh<int> crect, int enable_window_border, std::wstring _name,
				int doublebuffer, int _bpp) {
			int f = 1;

			enum flag {
				CAPTION = WS_CAPTION,
				MENU = CAPTION | WS_SYSMENU,
				ALL_WINDOW_ELEMENTS = CAPTION | MENU
			};

			menu = enable_window_border ? ALL_WINDOW_ELEMENTS : 0; bpp = _bpp; doublebuf = doublebuffer; name = _name.c_str();

			style =   menu ? (WS_OVERLAPPED | menu)|WS_CLIPSIBLINGS|WS_CLIPCHILDREN : WS_POPUP;
			exstyle = menu ? WS_EX_WINDOWEDGE : WS_EX_APPWINDOW; 
			errf((hwnd = CreateWindowEx(exstyle, L"AugmentedWindow", _name.c_str(), style, 0, 0, 0, 0, 0, 0, GetModuleHandle(NULL), this)), f);

			set_window_rect(crect);

			PIXELFORMATDESCRIPTOR p;
			ZeroMemory(&p, sizeof(p));

			p.nSize = sizeof(p);
			p.nVersion = 1;
			p.dwFlags =  PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | (doublebuffer?PFD_DOUBLEBUFFER:0);
			p.iPixelType = PFD_TYPE_RGBA;
			p.cColorBits = bpp;
			p.cAlphaBits = 8;
			p.cDepthBits = 16;
			p.iLayerType = PFD_MAIN_PLANE;
			errf(hdc = GetDC(hwnd), f);

			GLuint pf;
			errf(pf = ChoosePixelFormat(hdc, &p), f);
			errf(SetPixelFormat(hdc,pf,&p), f);

			errf(hglrc = wglCreateContext(hdc), f); glerr

			set_as_current();
			ShowWindow(hwnd, SW_SHOW);

			SetLastError(0);
			err(!(SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)this) == 0 && GetLastError() != 0));
			
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

			return f != 0;
		}

		bool glwindow::swap_buffers() {
			if(this != context) set_as_current();
			return err(SwapBuffers(hdc)) != FALSE;
		}

		bool glwindow::set_as_current() {
			bool ret = true;
			if(context != this) {
				ret = err(wglMakeCurrent(hdc, hglrc)) != FALSE; glerr
				context = this;
			}
			
			return ret;
		}
		 
		bool glwindow::set_vsync(int v) {
			bool ret = WGLEW_EXT_swap_control != NULL;
			errs(ret, "vsync not supported!");
			if(ret) {
				errs(ret = set_as_current(), "error enabling vsync, could not set current context");
				wglSwapIntervalEXT(v); glerr
				vsyn = v;
			}
			return ret;
		}
		
		bool glwindow::poll_event(event::message& out) {
			if(PeekMessageW(&wmsg, hwnd, 0, 0, PM_REMOVE)) {
				 //DispatchMessage(&wmsg); 
				out = events.msg = event::message(wmsg.message);
				_poll(out, wmsg.wParam, wmsg.lParam);
				
				TranslateMessage(&wmsg);

				return true;
			}
			return false;
		}

		std::vector<event::state> glwindow::poll_events() {
			window::event::message msg;
			std::vector<event::state> output;

			while (poll_event(msg)) {
				auto& state = glwindow::get_current()->events;

				if (!state.repeated)
					output.push_back(state);
			}
			
			if (GetFocus() == hwnd)
				enable_cursor_clipping(get_window_rect());
			else
				disable_cursor_clipping();
			
			return output;
		}

		bool glwindow::set_window_rect(const rects::xywh<int>& r) {
			static RECT wr = {0};
			int f = 1;
			errf(SetRect(&wr, r.x, r.y, r.r(), r.b()), f);
			errf(AdjustWindowRectEx(&wr, style, FALSE, exstyle), f);
			errf(MoveWindow(hwnd, wr.left, wr.top, wr.right-wr.left, wr.bottom-wr.top, TRUE), f);
			return f != 0;
		}

		rects::wh<int> glwindow::get_screen_rect() const {
			return rects::wh<int>(get_window_rect().w, get_window_rect().h);
		}

		rects::xywh<int> glwindow::get_window_rect() const {
			static RECT r;
			GetClientRect(hwnd, &r);
			ClientToScreen(hwnd, (POINT*)&r);
			ClientToScreen(hwnd, (POINT*)&r + 1);
			return rects::ltrb<int>(r.left, r.top, r.right, r.bottom);
		}

		bool glwindow::is_active() const {
			return active; 
		}

		void glwindow::destroy() {
			if(hwnd) {
				if(context == this) {
					wglMakeCurrent(NULL, NULL); glerr
					wglDeleteContext(hglrc); glerr
					context = 0;
				}
				ReleaseDC(hwnd, hdc);
				DestroyWindow(hwnd);
				hwnd = 0;
				hdc = 0;
				hglrc = 0;
			}
		}

		glwindow::~glwindow() {
			destroy();
		}
	}
}