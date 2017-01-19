#include "window.h"

#include <GL/OpenGL.h>
#include <algorithm>

#include "augs/log.h"
#include "augs/templates/string_templates.h"

#include "platform_utils.h"

#ifdef PLATFORM_WINDOWS
augs::window::event::message translate_enum(UINT m) {
	using namespace augs::window::event;

	switch (m) {
	case UINT(message::ltripleclick):	return message::ltripleclick;
	case SC_CLOSE:						return message::close; break;
	case WM_MOVE:						return message::move; break;
	case WM_ACTIVATE:					return message::activate; break;
	case SC_MINIMIZE:					return message::minimize; break;
	case SC_MAXIMIZE:					return message::maximize; break;
	case SC_RESTORE:					return message::restore;  break;
	case WM_CLIPBOARDUPDATE:			return message::clipboard_change; break;
	case WM_KEYDOWN:					return message::keydown; break;
	case WM_KEYUP:						return message::keyup; break;
	case WM_CHAR:						return message::character; break;
	case WM_UNICHAR:					return message::unichar; break;
	case WM_MOUSEMOVE:					return message::mousemotion; break;
	case WM_MOUSEWHEEL:					return message::wheel; break;
	case WM_LBUTTONDBLCLK:				return message::ldoubleclick; break;
	case WM_MBUTTONDBLCLK:				return message::mdoubleclick; break;
	case WM_XBUTTONDBLCLK:				return message::xdoubleclick; break;
	case WM_RBUTTONDBLCLK:				return message::rdoubleclick; break;
	case WM_LBUTTONDOWN:				return message::ldown; break;
	case WM_LBUTTONUP:					return message::lup; break;
	case WM_MBUTTONDOWN:				return message::mdown; break;
	case WM_MBUTTONUP:					return message::mup; break;
	case WM_XBUTTONDOWN:				return message::xdown; break;
	case WM_XBUTTONUP:					return message::xup; break;
	case WM_RBUTTONDOWN:				return message::rdown; break;
	case WM_RBUTTONUP:					return message::rup; break;
	default: break;
	}

	return augs::window::event::message::unknown;
}

augs::window::event::keys::key translate_key(UINT m) {
	using namespace augs::window::event::keys;

	switch (m) {
	case VK_LBUTTON:										return key::LMOUSE;
	case VK_RBUTTON:										return key::RMOUSE;
	case VK_MBUTTON:										return key::MMOUSE;
	case VK_XBUTTON1:										return key::MOUSE4;
	case VK_XBUTTON2:										return key::MOUSE5;
	case VK_CANCEL:											return key::CANCEL;
	case VK_BACK:											return key::BACKSPACE;
	case VK_TAB:											return key::TAB;
	case VK_CLEAR:											return key::CLEAR;
	case VK_RETURN:											return key::ENTER;
	case VK_SHIFT:											return key::SHIFT;
	case VK_CONTROL:										return key::CTRL;
	case VK_MENU:											return key::ALT;
	case VK_PAUSE:											return key::PAUSE;
	case VK_CAPITAL:										return key::CAPSLOCK;
	case VK_ESCAPE:											return key::ESC;
	case VK_SPACE:											return key::SPACE;
	case VK_PRIOR:											return key::PAGEUP;
	case VK_NEXT:											return key::PAGEDOWN;
	case VK_END:											return key::END;
	case VK_HOME:											return key::HOME;
	case VK_LEFT:											return key::LEFT;
	case VK_UP:												return key::UP;
	case VK_RIGHT:											return key::RIGHT;
	case VK_DOWN:											return key::DOWN;
	case VK_SELECT:											return key::SELECT;
	case VK_PRINT:											return key::PRINT;
	case VK_EXECUTE:										return key::EXECUTE;
	case VK_SNAPSHOT:										return key::PRINTSCREEN;
	case VK_INSERT:											return key::INSERT;
	case VK_DELETE:											return key::DEL;
	case VK_HELP:											return key::HELP;
	case VK_LWIN:											return key::LWIN;
	case VK_RWIN:											return key::RWIN;
	case VK_APPS:											return key::APPS;
	case VK_SLEEP:											return key::SLEEP;
	case VK_NUMPAD0:										return key::NUMPAD0;
	case VK_NUMPAD1:										return key::NUMPAD1;
	case VK_NUMPAD2:										return key::NUMPAD2;
	case VK_NUMPAD3:										return key::NUMPAD3;
	case VK_NUMPAD4:										return key::NUMPAD4;
	case VK_NUMPAD5:										return key::NUMPAD5;
	case VK_NUMPAD6:										return key::NUMPAD6;
	case VK_NUMPAD7:										return key::NUMPAD7;
	case VK_NUMPAD8:										return key::NUMPAD8;
	case VK_NUMPAD9:										return key::NUMPAD9;
	case VK_MULTIPLY:										return key::MULTIPLY;
	case VK_ADD:											return key::ADD;
	case VK_SEPARATOR:										return key::SEPARATOR;
	case VK_SUBTRACT:										return key::SUBTRACT;
	case VK_DECIMAL:										return key::DECIMAL;
	case VK_DIVIDE:											return key::DIVIDE;
	case VK_F1:												return key::F1;
	case VK_F2:												return key::F2;
	case VK_F3:												return key::F3;
	case VK_F4:												return key::F4;
	case VK_F5:												return key::F5;
	case VK_F6:												return key::F6;
	case VK_F7:												return key::F7;
	case VK_F8:												return key::F8;
	case VK_F9:												return key::F9;
	case VK_F10:											return key::F10;
	case VK_F11:											return key::F11;
	case VK_F12:											return key::F12;
	case VK_F13:											return key::F13;
	case VK_F14:											return key::F14;
	case VK_F15:											return key::F15;
	case VK_F16:											return key::F16;
	case VK_F17:											return key::F17;
	case VK_F18:											return key::F18;
	case VK_F19:											return key::F19;
	case VK_F20:											return key::F20;
	case VK_F21:											return key::F21;
	case VK_F22:											return key::F22;
	case VK_F23:											return key::F23;
	case VK_F24:											return key::F24;
	case 'A':												return key::A;
	case 'B':												return key::B;
	case 'C':												return key::C;
	case 'D':												return key::D;
	case 'E':												return key::E;
	case 'F':												return key::F;
	case 'G':												return key::G;
	case 'H':												return key::H;
	case 'I':												return key::I;
	case 'J':												return key::J;
	case 'K':												return key::K;
	case 'L':												return key::L;
	case 'M':												return key::M;
	case 'N':												return key::N;
	case 'O':												return key::O;
	case 'P':												return key::P;
	case 'Q':												return key::Q;
	case 'R':												return key::R;
	case 'S':												return key::S;
	case 'T':												return key::T;
	case 'U':												return key::U;
	case 'V':												return key::V;
	case 'W':												return key::W;
	case 'X':												return key::X;
	case 'Y':												return key::Y;
	case 'Z':												return key::Z;
	case '0':												return key::_0;
	case '1':												return key::_1;
	case '2':												return key::_2;
	case '3':												return key::_3;
	case '4':												return key::_4;
	case '5':												return key::_5;
	case '6':												return key::_6;
	case '7':												return key::_7;
	case '8':												return key::_8;
	case '9':												return key::_9;
	case VK_NUMLOCK:										return key::NUMLOCK;
	case VK_SCROLL:											return key::SCROLL;
	case VK_LSHIFT:											return key::LSHIFT;
	case VK_RSHIFT:											return key::RSHIFT;
	case VK_LCONTROL:										return key::LCTRL;
	case VK_RCONTROL:										return key::RCTRL;
	case VK_LMENU:											return key::LALT;
	case VK_RMENU:											return key::RALT;
	case VK_OEM_3:											return key::DASH;
	default:												return key::INVALID;
	}
}

namespace augs {
	extern HINSTANCE hinst;

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

		glwindow* glwindow::get_current() {
			return context;
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

			ensure(hglrc = wglCreateContext(hdc)); glerr

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

		bool glwindow::set_as_current() {
			bool ret = true;
			if (context != this) {
				ret = wglMakeCurrent(hdc, hglrc) != FALSE; glerr
					context = this;
			}

			return ret;
		}

		bool glwindow::set_vsync(const int v) {
			bool ret = WGLEW_EXT_swap_control != NULL;
			ensure(ret && "vsync not supported!");
			if (ret) {
				ensure(ret = set_as_current() && "error enabling vsync, could not set current context");
				wglSwapIntervalEXT(v); glerr
			}
			return ret;
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
				if (context == this) {
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
