#include "window.h"

#include <GL/OpenGL.h>
#include <algorithm>

#include "augs/log.h"
#include "augs/templates.h"

#include "augs/error/augs_error.h"
#include "platform_utils.h"

#ifdef PLATFORM_WINDOWS
augs::window::event::message translate_enum(UINT m) {
	switch (m) {
	case UINT(augs::window::event::message::ltripleclick): return augs::window::event::message::ltripleclick;
	case SC_CLOSE:					return augs::window::event::message::close; break;
	case WM_MOVE:					return augs::window::event::message::move; break;
	case WM_ACTIVATE:				return augs::window::event::message::activate; break;
	case SC_MINIMIZE:				return augs::window::event::message::minimize; break;
	case SC_MAXIMIZE:				return augs::window::event::message::maximize; break;
	case SC_RESTORE:				return augs::window::event::message::restore;  break;
	case WM_CLIPBOARDUPDATE:		return augs::window::event::message::clipboard_change; break;
	case WM_KEYDOWN:				return augs::window::event::message::keydown; break;
	case WM_KEYUP:					return augs::window::event::message::keyup; break;
	case WM_CHAR:					return augs::window::event::message::character; break;
	case WM_UNICHAR:				return augs::window::event::message::unichar; break;
	case WM_MOUSEMOVE:				return augs::window::event::message::mousemotion; break;
	case WM_MOUSEWHEEL:				return augs::window::event::message::wheel; break;
	case WM_LBUTTONDBLCLK:			return augs::window::event::message::ldoubleclick; break;
	case WM_MBUTTONDBLCLK:			return augs::window::event::message::mdoubleclick; break;
	case WM_XBUTTONDBLCLK:			return augs::window::event::message::xdoubleclick; break;
	case WM_RBUTTONDBLCLK:			return augs::window::event::message::rdoubleclick; break;
	case WM_LBUTTONDOWN:			return augs::window::event::message::ldown; break;
	case WM_LBUTTONUP:				return augs::window::event::message::lup; break;
	case WM_MBUTTONDOWN:			return augs::window::event::message::mdown; break;
	case WM_MBUTTONUP:				return augs::window::event::message::mup; break;
	case WM_XBUTTONDOWN:			return augs::window::event::message::xdown; break;
	case WM_XBUTTONUP:				return augs::window::event::message::xup; break;
	case WM_RBUTTONDOWN:			return augs::window::event::message::rdown; break;
	case WM_RBUTTONUP:				return augs::window::event::message::rup; break;
	default: break;
	}

	return augs::window::event::message::unknown;
}

augs::window::event::keys::key translate_key(UINT m) {
	switch (m) {
	case VK_LBUTTON:											return augs::window::event::keys::key::LMOUSE;
	case VK_RBUTTON:											return augs::window::event::keys::key::RMOUSE;
	case VK_MBUTTON:											return augs::window::event::keys::key::MMOUSE;
	case VK_XBUTTON1:											return augs::window::event::keys::key::MOUSE4;
	case VK_XBUTTON2:											return augs::window::event::keys::key::MOUSE5;
	case VK_CANCEL:											return augs::window::event::keys::key::CANCEL;
	case VK_BACK:											return augs::window::event::keys::key::BACKSPACE;
	case VK_TAB:											return augs::window::event::keys::key::TAB;
	case VK_CLEAR:											return augs::window::event::keys::key::CLEAR;
	case VK_RETURN:											return augs::window::event::keys::key::ENTER;
	case VK_SHIFT:											return augs::window::event::keys::key::SHIFT;
	case VK_CONTROL:											return augs::window::event::keys::key::CTRL;
	case VK_MENU:											return augs::window::event::keys::key::ALT;
	case VK_PAUSE:											return augs::window::event::keys::key::PAUSE;
	case VK_CAPITAL:											return augs::window::event::keys::key::CAPSLOCK;
	case VK_ESCAPE:											return augs::window::event::keys::key::ESC;
	case VK_SPACE:											return augs::window::event::keys::key::SPACE;
	case VK_PRIOR:											return augs::window::event::keys::key::PAGEUP;
	case VK_NEXT:											return augs::window::event::keys::key::PAGEDOWN;
	case VK_END:											return augs::window::event::keys::key::END;
	case VK_HOME:											return augs::window::event::keys::key::HOME;
	case VK_LEFT:											return augs::window::event::keys::key::LEFT;
	case VK_UP:											return augs::window::event::keys::key::UP;
	case VK_RIGHT:											return augs::window::event::keys::key::RIGHT;
	case VK_DOWN:											return augs::window::event::keys::key::DOWN;
	case VK_SELECT:											return augs::window::event::keys::key::SELECT;
	case VK_PRINT:											return augs::window::event::keys::key::PRINT;
	case VK_EXECUTE:											return augs::window::event::keys::key::EXECUTE;
	case VK_SNAPSHOT:											return augs::window::event::keys::key::PRINTSCREEN;
	case VK_INSERT:											return augs::window::event::keys::key::INSERT;
	case VK_DELETE:											return augs::window::event::keys::key::DEL;
	case VK_HELP:											return augs::window::event::keys::key::HELP;
	case VK_LWIN:											return augs::window::event::keys::key::LWIN;
	case VK_RWIN:											return augs::window::event::keys::key::RWIN;
	case VK_APPS:											return augs::window::event::keys::key::APPS;
	case VK_SLEEP:											return augs::window::event::keys::key::SLEEP;
	case VK_NUMPAD0:											return augs::window::event::keys::key::NUMPAD0;
	case VK_NUMPAD1:											return augs::window::event::keys::key::NUMPAD1;
	case VK_NUMPAD2:											return augs::window::event::keys::key::NUMPAD2;
	case VK_NUMPAD3:											return augs::window::event::keys::key::NUMPAD3;
	case VK_NUMPAD4:											return augs::window::event::keys::key::NUMPAD4;
	case VK_NUMPAD5:											return augs::window::event::keys::key::NUMPAD5;
	case VK_NUMPAD6:											return augs::window::event::keys::key::NUMPAD6;
	case VK_NUMPAD7:											return augs::window::event::keys::key::NUMPAD7;
	case VK_NUMPAD8:											return augs::window::event::keys::key::NUMPAD8;
	case VK_NUMPAD9:											return augs::window::event::keys::key::NUMPAD9;
	case VK_MULTIPLY:											return augs::window::event::keys::key::MULTIPLY;
	case VK_ADD:											return augs::window::event::keys::key::ADD;
	case VK_SEPARATOR:											return augs::window::event::keys::key::SEPARATOR;
	case VK_SUBTRACT:											return augs::window::event::keys::key::SUBTRACT;
	case VK_DECIMAL:											return augs::window::event::keys::key::DECIMAL;
	case VK_DIVIDE:											return augs::window::event::keys::key::DIVIDE;
	case VK_F1:											return augs::window::event::keys::key::F1;
	case VK_F2:											return augs::window::event::keys::key::F2;
	case VK_F3:											return augs::window::event::keys::key::F3;
	case VK_F4:											return augs::window::event::keys::key::F4;
	case VK_F5:											return augs::window::event::keys::key::F5;
	case VK_F6:											return augs::window::event::keys::key::F6;
	case VK_F7:											return augs::window::event::keys::key::F7;
	case VK_F8:											return augs::window::event::keys::key::F8;
	case VK_F9:											return augs::window::event::keys::key::F9;
	case VK_F10:											return augs::window::event::keys::key::F10;
	case VK_F11:											return augs::window::event::keys::key::F11;
	case VK_F12:											return augs::window::event::keys::key::F12;
	case VK_F13:											return augs::window::event::keys::key::F13;
	case VK_F14:											return augs::window::event::keys::key::F14;
	case VK_F15:											return augs::window::event::keys::key::F15;
	case VK_F16:											return augs::window::event::keys::key::F16;
	case VK_F17:											return augs::window::event::keys::key::F17;
	case VK_F18:											return augs::window::event::keys::key::F18;
	case VK_F19:											return augs::window::event::keys::key::F19;
	case VK_F20:											return augs::window::event::keys::key::F20;
	case VK_F21:											return augs::window::event::keys::key::F21;
	case VK_F22:											return augs::window::event::keys::key::F22;
	case VK_F23:											return augs::window::event::keys::key::F23;
	case VK_F24:											return augs::window::event::keys::key::F24;
	case 'A':											return augs::window::event::keys::key::A;
	case 'B':											return augs::window::event::keys::key::B;
	case 'C':											return augs::window::event::keys::key::C;
	case 'D':											return augs::window::event::keys::key::D;
	case 'E':											return augs::window::event::keys::key::E;
	case 'F':											return augs::window::event::keys::key::F;
	case 'G':											return augs::window::event::keys::key::G;
	case 'H':											return augs::window::event::keys::key::H;
	case 'I':											return augs::window::event::keys::key::I;
	case 'J':											return augs::window::event::keys::key::J;
	case 'K':											return augs::window::event::keys::key::K;
	case 'L':											return augs::window::event::keys::key::L;
	case 'M':											return augs::window::event::keys::key::M;
	case 'N':											return augs::window::event::keys::key::N;
	case 'O':											return augs::window::event::keys::key::O;
	case 'P':											return augs::window::event::keys::key::P;
	case 'Q':											return augs::window::event::keys::key::Q;
	case 'R':											return augs::window::event::keys::key::R;
	case 'S':											return augs::window::event::keys::key::S;
	case 'T':											return augs::window::event::keys::key::T;
	case 'U':											return augs::window::event::keys::key::U;
	case 'V':											return augs::window::event::keys::key::V;
	case 'W':											return augs::window::event::keys::key::W;
	case 'X':											return augs::window::event::keys::key::X;
	case 'Y':											return augs::window::event::keys::key::Y;
	case 'Z':											return augs::window::event::keys::key::Z;
	case '0':											return augs::window::event::keys::key::_0;
	case '1':											return augs::window::event::keys::key::_1;
	case '2':											return augs::window::event::keys::key::_2;
	case '3':											return augs::window::event::keys::key::_3;
	case '4':											return augs::window::event::keys::key::_4;
	case '5':											return augs::window::event::keys::key::_5;
	case '6':											return augs::window::event::keys::key::_6;
	case '7':											return augs::window::event::keys::key::_7;
	case '8':											return augs::window::event::keys::key::_8;
	case '9':											return augs::window::event::keys::key::_9;
	case VK_NUMLOCK:											return augs::window::event::keys::key::NUMLOCK;
	case VK_SCROLL:											return augs::window::event::keys::key::SCROLL;
	case VK_LSHIFT:											return augs::window::event::keys::key::LSHIFT;
	case VK_RSHIFT:											return augs::window::event::keys::key::RSHIFT;
	case VK_LCONTROL:											return augs::window::event::keys::key::LCTRL;
	case VK_RCONTROL:											return augs::window::event::keys::key::RCTRL;
	case VK_LMENU:											return augs::window::event::keys::key::LALT;
	case VK_RMENU:											return augs::window::event::keys::key::RALT;
	case VK_OEM_3:											return augs::window::event::keys::key::DASH;
	default: return augs::window::event::keys::key::INVALID;
	}
}

namespace augs {
	extern HINSTANCE hinst;

	namespace window {
		LRESULT CALLBACK wndproc (HWND hwnd, UINT umsg, WPARAM wParam, LPARAM lParam) {
			if(umsg == WM_GETMINMAXINFO || umsg == WM_INPUT)
				return DefWindowProc(hwnd, umsg, wParam, lParam);
			
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
					UINT in = umsg;
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
					if(events.character.utf16 > 255) {
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
					auto translated_key = translate_key(wParam);

					switch (translated_key) {
					case key::CTRL: translated_key = (lParam & 0x1000000) ? key::RCTRL : key::LCTRL; break;
					case key::SHIFT: translated_key = (lParam & 0x1000000) ? key::RSHIFT : key::LSHIFT; break;
					case key::ALT: translated_key = (lParam & 0x1000000) ? key::RALT : key::LALT; break;
					}

					events.key.key = translated_key;
					events.repeated = ((lParam & (1 << 30)) != 0);
				}
					break;								

				case WM_KEYUP:
				{
					auto translated_key = translate_key(wParam);
					switch (translated_key) {
					case key::CTRL: translated_key = (lParam & 0x1000000) ? key::RCTRL : key::LCTRL; break;
					case key::SHIFT: translated_key = (lParam & 0x1000000) ? key::RSHIFT : key::LSHIFT; break;
					case key::ALT: translated_key = (lParam & 0x1000000) ? key::RALT : key::LALT; break;
					}

					events.key.key = translated_key;
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
					if(GetCapture() == hwnd) ReleaseCapture(); break;
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
							events.mouse.rel.x = raw->data.mouse.lLastX;
							events.mouse.rel.y = raw->data.mouse.lLastY;

							m = WM_MOUSEMOVE;
						}
					}

					break;

				case WM_ACTIVATE:
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
					default: DefWindowProc(hwnd, m, wParam, lParam); break;
					}
					m = wParam;
					break;

				default: DefWindowProc(hwnd, m, wParam, lParam); break;
				}

				events.msg = translate_enum(m);
		}

		glwindow* glwindow::get_current() {
			return context;
		}

		glwindow* glwindow::context = nullptr;
		
		glwindow::glwindow() {
			triple_click_delay = GetDoubleClickTime();
		}

		int glwindow::create(rects::xywh<int> crect, int enable_window_border, std::string nn,
				int doublebuffer, int _bpp) {
			int f = 1;
			std::wstring _name(nn.begin(), nn.end());

			enum flag {
				CAPTION = WS_CAPTION,
				MENU = CAPTION | WS_SYSMENU,
				ALL_WINDOW_ELEMENTS = CAPTION | MENU
			};

			menu = enable_window_border ? ALL_WINDOW_ELEMENTS : 0; bpp = _bpp; doublebuf = doublebuffer; name = nn;

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
		
		bool glwindow::poll_event(UINT& out) {
			if(PeekMessageW(&wmsg, hwnd, 0, 0, PM_REMOVE)) {
				 //DispatchMessage(&wmsg); 
				out = wmsg.message;
				_poll(out, wmsg.wParam, wmsg.lParam);
				
				TranslateMessage(&wmsg);

				return true;
			}
			return false;
		}

		std::vector<event::change> glwindow::poll_events(const bool should_clip_cursor) {
			UINT msg;
			std::vector<event::change> output;

			while (poll_event(msg)) {
				auto& state = glwindow::get_current()->latest_change;

				if (!state.repeated)
					output.push_back(state);
			}
			
			if (should_clip_cursor && GetFocus() == hwnd)
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
#elif PLATFORM_LINUX
#endif
