#include "stdafx.h"
#include "../options.h"


#include "window.h"
#include "../misc/stream.h"
#include <algorithm>

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
					wnd->_poll(umsg, wParam, lParam);
					return 0;
				}
				else if(umsg == WM_SIZE) {
					LRESULT res = DefWindowProc(hwnd, umsg, wParam, lParam);
					if(wnd->resize) wnd->resize(*wnd);
					return res;
				}
			} 
			return DefWindowProc(hwnd, umsg, wParam, lParam);
		}
		
		void glwindow::_poll(event::message& m, WPARAM wParam, LPARAM lParam) {
				using namespace event::key;
				using namespace event::keys;

				static POINTS p;
				static RECT* r;
				static long rw, rh;
				static MINMAXINFO* mi;
				static BYTE lpb[40];
				static UINT dwSize = 40;
				static RAWINPUT* raw;

				events.mouse.rel.x = 0;
				events.mouse.rel.y = 0;
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

				case down:
					//if (!((lParam & (1 << 30)) != 0)) {
						events.keys[wParam] = true;
						switch(wParam) {
						case CTRL: wParam = (lParam & 0x1000000) ? RCTRL : LCTRL; break;
						case SHIFT: wParam = (lParam & 0x1000000) ? RSHIFT : LSHIFT; break;
						case ALT: wParam = (lParam & 0x1000000) ? RALT : LALT; break;
						}
						events.keys[wParam] = true;
						events.key = wParam;
						events.repeated = ((lParam & (1 << 30)) != 0);
					//}
					break;								

				case up:							
					events.keys[wParam] = false;
					switch(wParam) {
					case CTRL:	   events.keys[RCTRL] =		events.keys[LCTRL] = false; break;
					case SHIFT:	   events.keys[RSHIFT] =	events.keys[LSHIFT] = false; break;
					case ALT:	   events.keys[RALT] =		events.keys[LALT] = events.keys[LCTRL] = events.keys[CTRL] = false; break;
					}
					events.key = wParam;
					break;

					using namespace event::mouse;
				case wheel:
					events.mouse.scroll = GET_WHEEL_DELTA_WPARAM(wParam);
					break;
				case ldown:
						events.mouse.state[0] = events.keys[LMOUSE] = true;  
						if(doubled && triple_timer.extract<std::chrono::milliseconds>() < triple_click_delay) {
							m = events.msg = ltripleclick;
							doubled = false;
						}
						SetCapture(hwnd);
					break;
				case rdown:				    
					events.mouse.state[1] = events.keys[RMOUSE] = true;  break;
				case mdown:				    
					events.mouse.state[2] = events.keys[MMOUSE] = true;  break;
				case ldoubleclick:
					SetCapture(hwnd);
					events.mouse.state[0] = events.keys[LMOUSE] = true; 
					triple_timer.extract<std::chrono::microseconds>();
					doubled = true;
					break;
				case rdoubleclick:			    
					events.mouse.state[1] = events.keys[RMOUSE] = true;  break;
				case mdoubleclick:			    
					events.mouse.state[2] = events.keys[MMOUSE] = true;  break;
				case lup:				    
					events.mouse.state[0] = events.keys[LMOUSE] = false; if(GetCapture() == hwnd) ReleaseCapture(); break;
				case rup:				    
					events.mouse.state[1] = events.keys[RMOUSE] = false; break;
				case mup:				    
					events.mouse.state[2] = events.keys[MMOUSE] = false; break;
				case motion:
					p = MAKEPOINTS(lParam);
					events.mouse.rel.x = p.x - events.mouse.pos.x;
					events.mouse.rel.y = p.y - events.mouse.pos.y;
					if(events.mouse.rel.x || events.mouse.rel.y) doubled = false;
					events.mouse.pos.x = p.x;
					events.mouse.pos.y = p.y;

					if(!events.mouse.state[0]) {
						events.mouse.ldrag.x = events.mouse.pos.x;
						events.mouse.ldrag.y = events.mouse.pos.y;
					}

					if(!events.mouse.state[1]) {
						events.mouse.rdrag.x = events.mouse.pos.x;
						events.mouse.rdrag.y = events.mouse.pos.y;
					}
					break;

				case WM_INPUT:
					GetRawInputData(reinterpret_cast<HRAWINPUT>(lParam), RID_INPUT,
						lpb, &dwSize, sizeof(RAWINPUTHEADER));

					raw = reinterpret_cast<RAWINPUT*>(lpb);

					if (raw->header.dwType == RIM_TYPEMOUSE) {
						events.mouse.raw_rel.x = raw->data.mouse.lLastX;
						events.mouse.raw_rel.y = raw->data.mouse.lLastY;

						m = events.msg = raw_motion;
					}

					break;

				case event::activate:	
					active = (bool)(!HIWORD(wParam));
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
					m = events.msg = wParam;
					break;

				default: DefWindowProc(hwnd, events.msg, wParam, lParam); return;
				}
		}

		glwindow* glwindow::getcurrent() {
			return context;
		}

		glwindow* glwindow::context = nullptr;
		
		glwindow::glwindow()
			: hwnd(0), hdc(0), hglrc(0), bpp(0), resize(nullptr), menu(false), transparent(false), active(false), doubled(false) {
			for(int i=0;i<256;++i) events.keys[i] = false;
			events.key = events.utf16 = events.utf32 = 0;
			events.mouse.state[0] = events.mouse.state[1] = false;
			triple_click_delay = GetDoubleClickTime();
		}
		
		std::wstring to_wstr(const std::string& ss) {
			return std::wstring(ss.begin(), ss.end());
		}

		//bool glwindow::create(lua_State* L, rects::wh<int> force_minimum_resolution, int _menu) {
		//	luabind::object cfg = luabind::globals(L)["config_table"];
		//	luabind::object my_object = cfg["fullscreen"];
		//
		//	if (luabind::object_cast<int>(my_object)) {
		//		auto r = get_display();
		//		return create(rects::xywh<int>(0,0, r.w,r.h), 
		//			luabind::object_cast<int>(cfg["window_border"]) > 0 ? _menu : 0,
		//			to_wstr(luabind::object_cast<std::string>(cfg["window_name"])).c_str(),
		//			luabind::object_cast<int>(cfg["doublebuffer"]) > 0,
		//			luabind::object_cast<int>(cfg["bpp"])
		//			);
		//	}
		//	else {
		//		return create(rects::xywh<int>(luabind::object_cast<int>(cfg["window_x"]),
		//			luabind::object_cast<int>(cfg["window_y"]),
		//			std::max(force_minimum_resolution.w, int(luabind::object_cast<float>(cfg["resolution_w"]))),
		//			std::max(force_minimum_resolution.h, int(luabind::object_cast<float>(cfg["resolution_h"])))),
		//			luabind::object_cast<int>(cfg["window_border"]) > 0 ? _menu : 0,
		//			to_wstr(luabind::object_cast<std::string>(cfg["window_name"])).c_str(),
		//			luabind::object_cast<int>(cfg["doublebuffer"]) > 0,
		//			luabind::object_cast<int>(cfg["bpp"])
		//			);
		//	}
		//}

			int glwindow::create(rects::xywh<int> crect, int _menu, std::wstring _name,
				int doublebuffer, int _bpp) {
			int f = 1;
			menu = _menu; bpp = _bpp; doublebuf = doublebuffer; name = _name.c_str();

			style =   menu ? (WS_OVERLAPPED | menu)|WS_CLIPSIBLINGS|WS_CLIPCHILDREN : WS_POPUP;
			exstyle = menu ? WS_EX_WINDOWEDGE : WS_EX_APPWINDOW; 
			errf((hwnd = CreateWindowEx(exstyle, L"AugmentedWindow", _name.c_str(), style, 0, 0, 0, 0, 0, 0, hinst, this)), f);

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

			errf(hglrc = wglCreateContext(hdc), f);

			current();

			SetLastError(0);
			err(!(SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)this) == 0 && GetLastError() != 0));
			
			set_maximum_size(crect);
			set_minimum_size(crect);

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
			if(this != context) current();
			return err(SwapBuffers(hdc)) != FALSE;
		}

		bool glwindow::focus_keyboard() { 
			return err(SetFocus(hwnd)) != NULL;
		}

		bool glwindow::current() {
			bool ret = true;
			if(context != this) {
				ret = err(wglMakeCurrent(hdc, hglrc)) != FALSE;
				context = this;
			}
			return ret;
		}
		 
		bool glwindow::vsync(int v) {
			bool ret = WGLEW_EXT_swap_control != NULL;
			errs(ret, "vsync not supported!");
			if(ret) {
				errs(ret = current(), "error enabling vsync, could not set current context");
				wglSwapIntervalEXT(v);
				vsyn = v;
			}
			return ret;
		}
		
#ifdef INCLUDE_DWM
		bool glwindow::transparency(bool f) {
			transparent = f;
			DWM_BLURBEHIND bb = {0};
			bb.dwFlags = DWM_BB_ENABLE;
			bb.fEnable = f;
			bb.hRgnBlur = NULL;
			return err(DwmEnableBlurBehindWindow(hwnd, &bb) == S_OK);
		}
#endif

		bool glwindow::poll_events(event::message& out) {
			if(PeekMessageW(&wmsg, hwnd, 0, 0, PM_REMOVE)) {
				 //DispatchMessage(&wmsg); 
				using namespace event::key;
				out = events.msg = wmsg.message;
				_poll(out, wmsg.wParam, wmsg.lParam);
				
				if(out == event::minimize)
					set_show(MINIMIZE);
				if(out == event::maximize)
					set_show(MAXIMIZE);
				
				TranslateMessage(&wmsg);

				return true;
			}
			return false;
		}

		
		void glwindow::set_minimum_size(rects::wh<int> r) {
			if(!r.good()) r = get_window_rect();
			cminw = r.w;
			cminh = r.h;
			rects::xywh<int> rc = r;
			adjust(rc);
			minw = rc.w;
			minh = rc.h;
		}

		void glwindow::set_maximum_size(rects::wh<int> r) {
			if(!r.good()) r = get_window_rect();
			cmaxw = r.w;
			cmaxh = r.h;
			rects::xywh<int> rc = r;
			adjust(rc);
			maxw = rc.w;
			maxh = rc.h;
		}

		bool glwindow::set_window_rect(const rects::xywh<int>& r) {
			static RECT wr = {0};
			int f = 1;
			errf(SetRect(&wr, r.x, r.y, r.r(), r.b()), f);
			errf(AdjustWindowRectEx(&wr, style, FALSE, exstyle), f);
			errf(MoveWindow(hwnd, wr.left, wr.top, wr.right-wr.left, wr.bottom-wr.top, TRUE), f);
			return f != 0;
		}

		bool glwindow::set_adjusted_rect(const rects::xywh<int>& r) {
			return err(MoveWindow(hwnd, r.x, r.y, r.w, r.h, TRUE)) != FALSE;
		}

		bool glwindow::set_show(mode m) {
			return ShowWindow(hwnd, m) != FALSE;/*
											if(m == SHOW) {
											focus_keyboard();
											SetForegroundWindow(hwnd);
											UpdateWindow(hwnd);
											}*/
		}

		int glwindow::set_caption(const wchar_t* name) {
			return SetWindowText(hwnd, name);
		}

		rects::wh<int> glwindow::get_minimum_size() const {
			return rects::wh<int>(cminw, cminh);
		} 

		rects::wh<int> glwindow::get_maximum_size() const {
			return rects::wh<int>(cmaxw, cmaxh);
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

		rects::xywh<int> glwindow::get_adjusted_rect() const {
			static RECT r;
			GetWindowRect(hwnd, &r);
			return rects::ltrb<int>(r.left, r.top, r.right, r.bottom);
		}

		void glwindow::adjust(rects::xywh<int>& rc) {
			static RECT wr;
			SetRect(&wr,0,0,rc.w,rc.h);
			
			errs(AdjustWindowRectEx(&wr, style, FALSE, exstyle), "Failed to adjust window rect");

			rc = rects::ltrb<int>(wr.left, wr.top, wr.right, wr.bottom);
		}
		
		HWND glwindow::get_hwnd() const {
			return hwnd;
		}

		int glwindow::get_vsync() const { 
			return vsyn; 
		}

		bool glwindow::is_menu() const { 
			return menu != 0; 
		}

		bool glwindow::is_transparent() const { 
			return transparent; 
		}
		
		bool glwindow::is_active() const {
			return active; 
		}

		bool glwindow::is_doublebuffered() const {
			return active; 
		}

		void glwindow::destroy() {
			if(hwnd) {
				if(context == this) {
					wglMakeCurrent(NULL, NULL);
					wglDeleteContext(hglrc);
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

		bool set_display(int width, int height, int bpp) {
			static DEVMODE screen;						
			ZeroMemory(&screen,sizeof(screen));	
			screen.dmSize=sizeof(screen);		
			screen.dmPelsWidth	= width;		
			screen.dmPelsHeight	= height;		
			screen.dmBitsPerPel	= bpp;			
			screen.dmFields = DM_BITSPERPEL|DM_PELSWIDTH|DM_PELSHEIGHT;
			return ChangeDisplaySettings(&screen,CDS_FULLSCREEN) == DISP_CHANGE_SUCCESSFUL;
		}

		rects::xywh<int> get_display() {
			static RECT rc;
			GetWindowRect(GetDesktopWindow(), &rc);
			return rects::xywh<int>(rc.left,rc.top,rc.right-rc.left,rc.bottom-rc.top);
		}

		int get_refresh_rate() {
			DEVMODE lpDevMode;
			memset(&lpDevMode, 0, sizeof(DEVMODE));
			lpDevMode.dmSize = sizeof(DEVMODE);
			lpDevMode.dmDriverExtra = 0;

			return EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &lpDevMode) ? lpDevMode.dmDisplayFrequency : -1;
		}

		void warp_cursor(int x, int y) {
			SetCursorPos(x, y);
		}

		void set_cursor_visible(int flag) {
			ShowCursor(flag);
		}

		void mbx(const wchar_t* title, const wchar_t* content) { 
			MessageBox(0, content, title, MB_OK); 
		}

		void imbx(int title, int content) { 
			MessageBox(0, misc::wstr(content).c_str(), misc::wstr(title).c_str(), MB_OK); 
		}

		void smbx(std::wstring title, std::wstring content) { 
			MessageBox(0, content.c_str(), title.c_str(), MB_OK); 
		}

		void copy_clipboard (std::wstring& from) {
			OpenClipboard(0);
			EmptyClipboard();
			HGLOBAL h = GlobalAlloc(GMEM_DDESHARE, (from.length()+1)*sizeof(WCHAR));
			LPWSTR p = (LPWSTR)GlobalLock(h);

			for(unsigned i = 0; i < from.length(); ++i)
				p[i] = from[i];

			p[from.length()] = 0;

			SetClipboardData(CF_UNICODETEXT, h);
			GlobalUnlock(p);
			CloseClipboard();
		}
		
		bool is_newline(unsigned i) {
			return (i == 0x000A || i == 0x000D);
		}

		void paste_clipboard(std::wstring& to) {
			OpenClipboard(NULL);
			if(!IsClipboardFormatAvailable(CF_UNICODETEXT)) {
				CloseClipboard();
				return;
			}
			HANDLE clip0 = GetClipboardData(CF_UNICODETEXT);
			LPWSTR p = (LPWSTR)GlobalLock(clip0);
			size_t len = wcslen(p);
			to.clear();
			to.reserve(len);

			for(size_t i = 0; i < len; ++i) {
				to += p[i];
				if(is_newline(p[i]) && i < len-1 && is_newline(p[i+1])) ++i;
			}

			GlobalUnlock(clip0);
			CloseClipboard();
		}
	}
}