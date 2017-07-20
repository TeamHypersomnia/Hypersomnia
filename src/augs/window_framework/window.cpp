#include "window.h"

#include <algorithm>

#include "augs/log.h"
#include "augs/templates/string_templates.h"
#include "augs/templates/corresponding_field.h"

#include "platform_utils.h"

#ifdef PLATFORM_WINDOWS
#include "augs/window_framework/translate_windows_enums.h"

namespace augs {
	vec2i window_settings::get_screen_size() const {
		return fullscreen ? augs::get_display().get_size() : size;
	}
	
	LRESULT CALLBACK wndproc(HWND hwnd, UINT umsg, WPARAM wParam, LPARAM lParam) {
		if (umsg == WM_GETMINMAXINFO || umsg == WM_INPUT) {
			return DefWindowProc(hwnd, umsg, wParam, lParam);
		}

		if (umsg == WM_CREATE) {
			AddClipboardFormatListener(hwnd);
		}

		if (umsg == WM_DESTROY) {
			RemoveClipboardFormatListener(hwnd);
		}
		else if (umsg == WM_SYSCOMMAND || umsg == WM_ACTIVATE || umsg == WM_INPUT) {
			(PostMessage(hwnd, umsg, wParam, lParam));
			return 0;
		}
		else if (umsg == WM_SIZE) {
			LRESULT res = DefWindowProc(hwnd, umsg, wParam, lParam);
			return res;
		}

		return DefWindowProc(hwnd, umsg, wParam, lParam);
	}

	std::optional<event::change> window::handle_event(const UINT m, const WPARAM wParam, const LPARAM lParam) {
		using namespace event::keys;

		thread_local MINMAXINFO* mi;
		thread_local BYTE lpb[40];
		thread_local UINT dwSize = 40;
		thread_local RAWINPUT* raw;

		event::change change;
		change.msg = translate_enum(m);

		switch (m) {
		case WM_CHAR:
			change.character.utf16 = wchar_t(wParam);
			if (change.character.utf16 > 255) {
				break;
			}
			//change.utf32 = unsigned(wParam);
			change.repeated = ((lParam & (1 << 30)) != 0);
			break;
		case WM_UNICHAR:
			//change.utf32 = unsigned(wParam);
			//change.repeated = ((lParam & (1 << 30)) != 0);
			break;

		case SC_CLOSE:
			break;

		case WM_KEYUP:
			change.key.key = translate_key_with_lparam(lParam, wParam);
			break;
		case WM_KEYDOWN:
			change.key.key = translate_key_with_lparam(lParam, wParam);
			change.repeated = ((lParam & (1 << 30)) != 0);
			break;

		case WM_SYSKEYUP:
			change.key.key = translate_key_with_lparam(lParam, wParam);
			break;
		case WM_SYSKEYDOWN:
			change.key.key = translate_key_with_lparam(lParam, wParam);
			change.repeated = ((lParam & (1 << 30)) != 0);
			break;

		case WM_MOUSEWHEEL:
			change.scroll.amount = GET_WHEEL_DELTA_WPARAM(wParam) / WHEEL_DELTA;
			break;
		case WM_LBUTTONDBLCLK:
		case WM_LBUTTONDOWN:
			change.key.key = key::LMOUSE;
			SetCapture(hwnd);

			if (m == WM_LBUTTONDOWN) {
				if (double_click_occured && triple_click_timer.extract<std::chrono::milliseconds>() < triple_click_delay) {
					change.msg = event::message::ltripleclick;
					double_click_occured = false;
				}
			}
			else {
				triple_click_timer.extract<std::chrono::microseconds>();
				double_click_occured = true;
			}

			break;
		case WM_RBUTTONDBLCLK:
		case WM_RBUTTONDOWN:
			change.key.key = key::RMOUSE;
			break;
		case WM_MBUTTONDBLCLK:
		case WM_MBUTTONDOWN:
			change.key.key = key::MMOUSE;
			break;
		case WM_XBUTTONDBLCLK:
		case WM_XBUTTONDOWN:
			change.key.key = key::MOUSE4;
			change.msg = event::message::keydown;
			break;
		case WM_XBUTTONUP:
			change.key.key = key::MOUSE4;
			change.msg = event::message::keyup;
			break;
		case WM_LBUTTONUP:
			change.key.key = key::LMOUSE;
			if (GetCapture() == hwnd) ReleaseCapture(); break;
		case WM_RBUTTONUP:
			change.key.key = key::RMOUSE;
			break;
		case WM_MBUTTONUP:
			change.key.key = key::MMOUSE;
			break;
		case WM_MOUSEMOVE:
			if (!raw_mouse_input) {
				vec2t<short> new_pos;

				{
					const auto p = MAKEPOINTS(lParam);
					new_pos = { p.x, p.y };
				}

				change.mouse.rel = new_pos - last_mouse_pos;
				
				if (change.mouse.rel.non_zero()) {
					double_click_occured = false;
				}

				last_mouse_pos = new_pos;
			}
			else {
				return std::nullopt;
			}

			break;

		case WM_INPUT:
			if (raw_mouse_input) {
				GetRawInputData(
					reinterpret_cast<HRAWINPUT>(lParam), 
					RID_INPUT,
					lpb, 
					&dwSize, 
					sizeof(RAWINPUTHEADER)
				);

				raw = reinterpret_cast<RAWINPUT*>(lpb);

				if (raw->header.dwType == RIM_TYPEMOUSE) {
					change.mouse.rel = {
						static_cast<short>(raw->data.mouse.lLastX),
						static_cast<short>(raw->data.mouse.lLastY)
					};

					if (!frozen) {
						last_mouse_pos += change.mouse.rel;
					}

					const auto screen_size = current_settings.get_screen_size() - vec2i(1, 1);
					last_mouse_pos.clamp_from_zero_to(screen_size);
					change.mouse.pos = vec2t<short>(last_mouse_pos);
					change.msg = translate_enum(WM_MOUSEMOVE);
				}
			}
			else {
				return std::nullopt;
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
			DefWindowProc(hwnd, m, wParam, lParam);
			change.msg = translate_enum(wParam);
			break;

		default: DefWindowProc(hwnd, m, wParam, lParam); break;
		}

		return change;
	}

	void window::set_window_name(const std::string& name) {
		SetWindowText(hwnd, to_wstring(name).c_str());
	}

	window::window(
		const window_settings& settings
	) {
		triple_click_delay = GetDoubleClickTime();

		if (!window_class_registered) {
			WNDCLASSEX wcl = { 0 };
			wcl.cbSize = sizeof(wcl);
			wcl.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
			wcl.lpfnWndProc = wndproc;
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

		ensure((hwnd = CreateWindowEx(0, L"AugmentedWindow", L"invalid_name", 0, 0, 0, 0, 0, 0, 0, GetModuleHandle(NULL), this)));

		PIXELFORMATDESCRIPTOR p;
		ZeroMemory(&p, sizeof(p));

		p.nSize = sizeof(p);
		p.nVersion = 1;
		p.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
		p.iPixelType = PFD_TYPE_RGBA;
		p.cColorBits = settings.bpp;
		p.cAlphaBits = 8;
		p.cDepthBits = 16;
		p.iLayerType = PFD_MAIN_PLANE;
		ensure(hdc = GetDC(hwnd));

		GLuint pf;
		ensure(pf = ChoosePixelFormat(hdc, &p));
		ensure(SetPixelFormat(hdc, pf, &p));

		ensure(hglrc = wglCreateContext(hdc));

		set_as_current();
		show();

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

		apply(settings, true);
		last_mouse_pos = settings.get_screen_size() / 2;
	}

	window_settings window::get_current_settings() const {
		return current_settings;
	}

	void window::apply(const window_settings& settings, const bool force) {
		auto changed = [&](auto& field) {
			return !(field == get_corresponding_field(field, settings, current_settings));
		};

		if (force || changed(settings.name)) {
			set_window_name(settings.name);
		}

		if (force || changed(settings.position)) {
			auto r = get_window_rect();
			r.set_position(settings.position);
			set_window_rect(r);
		}

		if (force || changed(settings.enable_cursor_clipping)) {
			set_cursor_visible(!settings.fullscreen && !settings.enable_cursor_clipping);
		}

		if (force || changed(settings.border)) {
			set_window_border_enabled(!settings.fullscreen && settings.border);
		}

		if (force || changed(settings.fullscreen)) {
			if (settings.fullscreen) {
				set_window_border_enabled(false);
				set_cursor_visible(false);

				set_display(settings.get_screen_size(), settings.bpp);
			}
			else {
				set_window_border_enabled(settings.border);
				set_cursor_visible(!settings.enable_cursor_clipping);
			}
		}

		if (force
			|| settings.get_screen_size() != current_settings.get_screen_size()
			|| changed(settings.border)
		) {
			xywhi screen_rect;

			if (!settings.fullscreen) {
				screen_rect.set_position(settings.position);
			}

			screen_rect.set_size(settings.get_screen_size());
			set_window_rect(screen_rect);
		}

		current_settings = settings;
	}

	void window::set_window_border_enabled(const bool f) {
		const auto menu = f ? WS_CAPTION | WS_SYSMENU : 0;

		style = menu ? (WS_OVERLAPPED | menu) | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_THICKFRAME | WS_SYSMENU | WS_MAXIMIZEBOX | WS_MINIMIZEBOX : WS_POPUP;
		exstyle = menu ? WS_EX_WINDOWEDGE : WS_EX_APPWINDOW;

		SetWindowLongPtr(hwnd, GWL_EXSTYLE, exstyle);
		SetWindowLongPtr(hwnd, GWL_STYLE, style);

		SetWindowPos(hwnd, NULL, 0, 0, 0, 0, SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOZORDER | SWP_NOOWNERZORDER);
		set_window_rect(get_window_rect());
		show();
	}

	bool window::swap_buffers() {
		return SwapBuffers(hdc) != FALSE;
	}

	void window::show() {
		ShowWindow(hwnd, SW_SHOW);
	}
	
	void window::set_mouse_position_frozen(const bool flag) {
		frozen = flag;
	}

	bool window::set_as_current_impl() {
		return wglMakeCurrent(hdc, hglrc);
	}

	void window::set_current_to_none_impl() {
		wglMakeCurrent(NULL, NULL);
	}

	std::vector<event::change> window::collect_entropy() {
		ensure(is_current());

		std::vector<event::change> output;

		while (PeekMessageW(&wmsg, hwnd, 0, 0, PM_REMOVE)) {
			const auto new_change = handle_event(
				wmsg.message, 
				wmsg.wParam, 
				wmsg.lParam
			);

			TranslateMessage(&wmsg);

			if (new_change.has_value() && !new_change->repeated) {
				output.push_back(*new_change);
			}
		}

		if (current_settings.enable_cursor_clipping && GetFocus() == hwnd) {
			enable_cursor_clipping(get_window_rect());
		}
		else {
			disable_cursor_clipping();
		}

		if (!current_settings.fullscreen) {
			current_settings.size = get_window_rect().get_size();
			current_settings.position = get_window_rect().get_position();
		}

		return output;
	}

	void window::set_window_rect(const xywhi r) {
		static RECT wr = { 0 };
		ensure(SetRect(&wr, r.x, r.y, r.r(), r.b()));
		ensure(AdjustWindowRectEx(&wr, style, FALSE, exstyle));
		ensure(MoveWindow(hwnd, wr.left, wr.top, wr.right - wr.left, wr.bottom - wr.top, TRUE));
	}

	vec2i window::get_screen_size() const {
		return get_window_rect().get_size();
	}

	xywhi window::get_window_rect() const {
		static RECT r;
		GetClientRect(hwnd, &r);
		ClientToScreen(hwnd, (POINT*)&r);
		ClientToScreen(hwnd, (POINT*)&r + 1);
		return ltrbi(r.left, r.top, r.right, r.bottom);
	}

	bool window::is_active() const {
		return active;
	}

	void window::destroy() {
		if (hwnd) {
			unset_if_current();

			wglDeleteContext(hglrc);
			ReleaseDC(hwnd, hdc);
			DestroyWindow(hwnd);

			hwnd = nullptr;
		}
	}

	window::~window() {
		destroy();
	}
}

bool augs::window::window_class_registered = false;

#elif PLATFORM_LINUX
#endif
