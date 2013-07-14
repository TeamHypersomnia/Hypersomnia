#define UNICODE
#define GLEW_STATIC
#include <gl/glew.h>
#include <gl/wglew.h>
#include <GL/GL.h>

#ifdef INCLUDE_DWM
#include <dwmapi.h>
#endif

#include "window.h"
#include "../config/config.h"
#include "../utility/stream.h"
#include <algorithm>

namespace augmentations {
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
				else if(umsg == WM_SYSCOMMAND || umsg == WM_ACTIVATE) {
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
					//if(!(lParam & 0x40000000)) {
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
						if(doubled && triple_timer.miliseconds() < triple_click_delay) {
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
					triple_timer.microseconds();
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
			: hwnd(0), hdc(0), hglrc(0), name(nullptr), bpp(0), resize(nullptr), menu(false), transparent(false), active(false), doubled(false) {
			for(int i=0;i<256;++i) events.keys[i] = false;
			events.key = events.utf16 = events.utf32 = 0;
			events.mouse.state[0] = events.mouse.state[1] = false;
			triple_click_delay = GetDoubleClickTime();
		}
		
		bool glwindow::create(const config::input_file& cfg, rects::wh force_minimum_resolution, int _menu) {
			if(cfg.values.at(L"fullscreen").int_val > 0) {
				auto r = get_display();
				return create(rects::xywh(0,0, r.w,r.h), 
					cfg.values.at(L"window_border").int_val > 0 ? _menu : 0,
					cfg.values.at(L"window_name").string_val.c_str(),
					cfg.values.at(L"doublebuffer").int_val > 0,
					cfg.values.at(L"bpp").int_val
					);
			}
			else {
				return create(rects::xywh(	cfg.values.at(L"window_x").int_val,
					cfg.values.at(L"window_y").int_val, 
					std::max(force_minimum_resolution.w, cfg.values.at(L"resolution_w").int_val),
					std::max(force_minimum_resolution.h, cfg.values.at(L"resolution_h").int_val)), 
					cfg.values.at(L"window_border").int_val > 0 ? _menu : 0,
					cfg.values.at(L"window_name").string_val.c_str(),
					cfg.values.at(L"doublebuffer").int_val > 0,
					cfg.values.at(L"bpp").int_val
					);
			}
		}

		bool glwindow::create(const rects::xywh& crect, int _menu, const wchar_t* _name, 
				bool doublebuffer, int _bpp) {
			int f = 1;
			menu = _menu; bpp = _bpp; doublebuf = doublebuffer; name = _name;

			style =   menu ? (WS_OVERLAPPED | menu)|WS_CLIPSIBLINGS|WS_CLIPCHILDREN : WS_POPUP;
			exstyle = menu ? WS_EX_WINDOWEDGE : WS_EX_APPWINDOW; 
			errf((hwnd = CreateWindowEx(exstyle,L"AugmentedWindow",name,style,0,0,0,0,0,0,hinst,this)), f);

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

		
		void glwindow::set_minimum_size(rects::wh r) {
			if(!r.good()) r = get_window_rect();
			cminw = r.w;
			cminh = r.h;
			rects::xywh rc = r;
			adjust(rc);
			minw = rc.w;
			minh = rc.h;
		}

		void glwindow::set_maximum_size(rects::wh r) {
			if(!r.good()) r = get_window_rect();
			cmaxw = r.w;
			cmaxh = r.h;
			rects::xywh rc = r;
			adjust(rc);
			maxw = rc.w;
			maxh = rc.h;
		}

		bool glwindow::set_window_rect(const rects::xywh& r) {
			static RECT wr = {0};
			int f = 1;
			errf(SetRect(&wr, r.x, r.y, r.r(), r.b()), f);
			errf(AdjustWindowRectEx(&wr, style, FALSE, exstyle), f);
			errf(MoveWindow(hwnd, wr.left, wr.top, wr.right-wr.left, wr.bottom-wr.top, TRUE), f);
			return f != 0;
		}

		bool glwindow::set_adjusted_rect(const rects::xywh& r) {
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

		rects::wh glwindow::get_minimum_size() const {
			return rects::wh(cminw, cminh);
		} 

		rects::wh glwindow::get_maximum_size() const {
			return rects::wh(cmaxw, cmaxh);
		}
		
		rects::xywh glwindow::get_window_rect() const {
			static RECT r;
			GetClientRect(hwnd, &r);
			ClientToScreen(hwnd, (POINT*)&r);
			ClientToScreen(hwnd, (POINT*)&r + 1);
			return rects::ltrb(r.left, r.top, r.right, r.bottom);
		}

		rects::xywh glwindow::get_adjusted_rect() const {
			static RECT r;
			GetWindowRect(hwnd, &r);
			return rects::ltrb(r.left, r.top, r.right, r.bottom);
		}

		void glwindow::adjust(rects::xywh& rc) {
			static RECT wr;
			SetRect(&wr,0,0,rc.w,rc.h);
			
			errs(AdjustWindowRectEx(&wr, style, FALSE, exstyle), "Failed to adjust window rect");

			rc = rects::ltrb(wr.left, wr.top, wr.right, wr.bottom);
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

		rects::xywh get_display() {
			static RECT rc;
			GetWindowRect(GetDesktopWindow(), &rc);
			return rects::xywh(rc.left,rc.top,rc.right-rc.left,rc.bottom-rc.top);
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

		void cursor(bool flag) {
			ShowCursor(flag);
		}


		void mbx(const wchar_t* title, const wchar_t* content) { 
			MessageBox(0, content, title, MB_OK); 
		}

		void imbx(int title, int content) { 
			MessageBox(0, util::wstr(content).c_str(), util::wstr(title).c_str(), MB_OK); 
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