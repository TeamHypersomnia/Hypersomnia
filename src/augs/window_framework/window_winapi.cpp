#include <algorithm>

#include "augs/log.h"

#include "augs/string/string_templates.h"
#include "augs/templates/corresponding_field.h"
#include "augs/templates/algorithm_templates.h"

#include "augs/window_framework/window.h"
#include "augs/window_framework/platform_utils.h"

#include "augs/window_framework/translate_winapi_enums.h"

namespace augs {
	std::wstring widen(const std::string& s) {
		return std::wstring(s.begin(), s.end());
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

		event::change change;
		change.msg = translate_enum(m);

		auto default_proc = [&]() {
			DefWindowProc(hwnd, m, wParam, lParam);
		};

		switch (m) {
		case WM_CHAR:
			change.data.character.code_point = wchar_t(wParam);
			if (change.data.character.code_point > 255) {
				return change;
			}
			//change.utf32 = unsigned(wParam);
			if (/* repeated */ ((lParam & (1 << 30)) != 0)) {
				return std::nullopt;
			}

			return change;
		case WM_UNICHAR:
			//change.utf32 = unsigned(wParam);
			//change.repeated = ((lParam & (1 << 30)) != 0);
			return change;

		case SC_CLOSE:
			return change;

		case WM_KEYUP:
			change.data.key.key = translate_key_with_lparam(lParam, wParam);
			return change;
		case WM_KEYDOWN:
			change.data.key.key = translate_key_with_lparam(lParam, wParam);

			if (/* repeated */ ((lParam & (1 << 30)) != 0)) {
				return std::nullopt;
			}

			return change;

		case WM_SYSKEYUP:
			change.data.key.key = translate_key_with_lparam(lParam, wParam);
			return change;
		case WM_SYSKEYDOWN:
			change.data.key.key = translate_key_with_lparam(lParam, wParam);

			if (/* repeated */ ((lParam & (1 << 30)) != 0)) {
				return std::nullopt;
			}

			return change;

		case WM_MOUSEWHEEL:
			change.data.scroll.amount = GET_WHEEL_DELTA_WPARAM(wParam) / WHEEL_DELTA;
			return change;
		
		// Doubleclicks
		
		case WM_LBUTTONDBLCLK:
			SetCapture(hwnd);
			
			triple_click_timer.extract<std::chrono::microseconds>();
			double_click_occured = true;

			return change;

		case WM_RBUTTONDBLCLK:
		case WM_MBUTTONDBLCLK:
			return change;

		// Downs
		
		case WM_LBUTTONDOWN:
			change.data.key.key = key::LMOUSE;

			SetCapture(hwnd);

			if (double_click_occured && triple_click_timer.extract<std::chrono::milliseconds>() < triple_click_delay) {
				change.msg = event::message::ltripleclick;
				double_click_occured = false;
			}

			return change;

		case WM_RBUTTONDOWN:
			change.data.key.key = key::RMOUSE;
			return change;
		case WM_MBUTTONDOWN:
			change.data.key.key = key::MMOUSE;
			return change;

		// dont support x double click already
		case WM_XBUTTONDBLCLK:
		case WM_XBUTTONDOWN:
			{
				const auto fwButton = GET_XBUTTON_WPARAM(wParam);

				if (fwButton == XBUTTON1) {
					change.data.key.key = key::MOUSE4;
				}
				else if(fwButton == XBUTTON2) {
					change.data.key.key = key::MOUSE5;
				}
				else {
					throw window_error("Unknown fwButton: %x", fwButton);
				}
			}

			return change;
		
		// Ups

		case WM_XBUTTONUP:
			{
				const auto fwButton = GET_XBUTTON_WPARAM(wParam);

				if (fwButton == XBUTTON1) {
					change.data.key.key = key::MOUSE4;
				}
				else if(fwButton == XBUTTON2) {
					change.data.key.key = key::MOUSE5;
				}
				else {
					throw window_error("Unknown fwButton: %x", fwButton);
				}
			}
			return change;
		case WM_LBUTTONUP:
			change.data.key.key = key::LMOUSE;
			if (GetCapture() == hwnd) ReleaseCapture(); return change;
		case WM_RBUTTONUP:
			change.data.key.key = key::RMOUSE;
			return change;
		case WM_MBUTTONUP:
			change.data.key.key = key::MMOUSE;
			return change;
		case WM_MOUSEMOVE:
			{
				const auto p = MAKEPOINTS(lParam);
				const auto new_pos = basic_vec2<short>{ p.x, p.y };

				double_click_occured = false;

				return handle_mousemove(new_pos);
			}

			return change;

		case WM_INPUT:
			if (is_active() && (current_settings.raw_mouse_input || mouse_pos_paused)) {
				thread_local BYTE lpb[sizeof(RAWINPUT)];
				thread_local UINT dwSize = sizeof(RAWINPUT);

				GetRawInputData(
					reinterpret_cast<HRAWINPUT>(lParam), 
					RID_INPUT,
					lpb, 
					&dwSize, 
					sizeof(RAWINPUTHEADER)
				);

				const auto* const raw = reinterpret_cast<RAWINPUT*>(lpb);

				if (raw->header.dwType == RIM_TYPEMOUSE) {
					return do_raw_motion({
						static_cast<short>(raw->data.mouse.lLastX),
						static_cast<short>(raw->data.mouse.lLastY)
					});
				}
			}

			return std::nullopt;

		case WM_ACTIVATE:
			{
				const auto type = LOWORD(wParam);

				switch (type) {
				case WA_INACTIVE: change.msg = event::message::deactivate; return change;
				case WA_ACTIVE: change.msg = event::message::activate; return change;
				case WA_CLICKACTIVE: change.msg = event::message::click_activate; return change;
				default: return std::nullopt;
				}

				if (change.msg == event::message::deactivate && current_settings.raw_mouse_input) {
					set_cursor_pos(last_mouse_pos);
				}
			}

			return change;

		case WM_GETMINMAXINFO:
			{
				auto* const mi = reinterpret_cast<MINMAXINFO*>(lParam);
				mi->ptMinTrackSize.x = min_window_size.x;
				mi->ptMinTrackSize.y = min_window_size.y;
				mi->ptMaxTrackSize.x = max_window_size.x;
				mi->ptMaxTrackSize.y = max_window_size.y;
			}
			return change;

		case WM_SYSCOMMAND:
			default_proc();
			change.msg = translate_enum(static_cast<UINT>(wParam));
			return change;

		default: default_proc(); return std::nullopt;
		}
	}

	void window::set_window_name(const std::string& name) {
		SetWindowText(hwnd, widen(name).c_str());
	}

	window::window(
		const window_settings& settings
	) {
		// TODO: throw an exception instead of ensuring
		static bool register_once = [](){
			WNDCLASSEX wcl = { 0 };
			wcl.cbSize = sizeof(wcl);
			wcl.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
			wcl.lpfnWndProc = wndproc;
			wcl.cbClsExtra = 0;
			wcl.cbWndExtra = 0;
			wcl.hInstance = GetModuleHandle(NULL);
#if ADD_APPLICATION_ICON
			wcl.hIcon = 0;
			wcl.hIconSm = 0;
#else
			wcl.hIcon = LoadIcon(0, IDI_APPLICATION);
			wcl.hIconSm = 0;
#endif
			wcl.hCursor = LoadCursor(0, IDC_ARROW);
			wcl.hbrBackground = 0;
			wcl.lpszMenuName = 0;
			wcl.lpszClassName = L"augwin";

			const auto register_success = RegisterClassEx(&wcl) != 0;

			ensure(register_success && "class registering");
			return register_success;
		}();

		triple_click_delay = GetDoubleClickTime();
		hwnd = CreateWindowEx(0, L"augwin", L"invalid_name", 0, 0, 0, 0, 0, 0, 0, GetModuleHandle(NULL), this);
		ensure(hwnd);

		PIXELFORMATDESCRIPTOR p;
		ZeroMemory(&p, sizeof(p));

		p.nSize = sizeof(p);
		p.nVersion = 1;
		p.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
		p.iPixelType = PFD_TYPE_RGBA;
		p.cColorBits = settings.bpp;
		p.cAlphaBits = 8;
		p.cDepthBits = 0;
		p.iLayerType = PFD_MAIN_PLANE;
		hdc = GetDC(hwnd);
		ensure(hdc);

		const auto pf = ChoosePixelFormat(hdc, &p);
		
		ensure(pf);

		{
			const auto result = SetPixelFormat(hdc, pf, &p);
			ensure(result);
		}

#if BUILD_OPENGL
		hglrc = wglCreateContext(hdc);
		ensure(hglrc);
#endif
		const auto sc = set_as_current();
		ensure(sc);
		show();

		SetLastError(0);
		{
			const auto result = !(SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)this) == 0 && GetLastError() != 0);
			ensure(result);
		}

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
		last_mouse_pos = get_screen_size() / 2;

		const auto app_icon_path = settings.app_icon_path.wstring();
		
		if (/* icon_was_specified */ !app_icon_path.empty()) {
			const auto app_icon = (HICON)LoadImage(NULL,
				app_icon_path.c_str(),
				IMAGE_ICON,       
				0,                
				0,                
				LR_LOADFROMFILE |  
				LR_DEFAULTSIZE |   
				LR_SHARED         
			);

			SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)app_icon);
			SendMessage(hwnd, WM_SETICON, ICON_BIG, (LPARAM)app_icon);
		}
	}

	void window::destroy() {
		if (hwnd) {
			unset_if_current();

#if BUILD_OPENGL
			wglDeleteContext(hglrc);
#endif
			ReleaseDC(hwnd, hdc);
			DestroyWindow(hwnd);

			hwnd = nullptr;
		}
	}

	void window::set_window_border_enabled(const bool enabled) {
		if (enabled) {
			style = WS_OVERLAPPEDWINDOW;
			exstyle = WS_EX_WINDOWEDGE;
		}
		else {
			style = WS_POPUP;
			exstyle = WS_EX_APPWINDOW;
		}

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

	bool window::set_as_current_impl() {
#if BUILD_OPENGL
		return wglMakeCurrent(hdc, hglrc);
#else
		return true;
#endif
	}

	void window::set_current_to_none_impl() {
#if BUILD_OPENGL
		wglMakeCurrent(NULL, NULL);
#endif
	}

	void window::collect_entropy(local_entropy& output) {
		ensure(is_current());

		{
			MSG wmsg;

			while (PeekMessageW(&wmsg, hwnd, 0, 0, PM_REMOVE)) {
				const auto new_change = handle_event(
					wmsg.message, 
					wmsg.wParam, 
					wmsg.lParam
				);

				TranslateMessage(&wmsg);

				if (new_change.has_value()) {
					common_event_handler(*new_change, output);
					output.push_back(*new_change);
				}
			}
		}
	}

	void window::set_window_rect(const xywhi r) {
		static RECT wr = { 0 };
		const auto result = 
			(SetRect(&wr, r.x, r.y, r.r(), r.b())) &&
			(AdjustWindowRectEx(&wr, style, FALSE, exstyle)) &&
			(MoveWindow(hwnd, wr.left, wr.top, wr.right - wr.left, wr.bottom - wr.top, TRUE))
		;

		ensure(result);
	}

	xywhi window::get_window_rect() const {
		static RECT r;
		GetClientRect(hwnd, &r);
		ClientToScreen(hwnd, (POINT*)&r);
		ClientToScreen(hwnd, (POINT*)&r + 1);
		return ltrbi(r.left, r.top, r.right, r.bottom);
	}

	void window::set_fullscreen_hint(const bool flag) {
		
	}

	static auto get_filter(const std::vector<window::file_dialog_filter>& filters) {
		std::wstring filter;

		auto to_reserve = std::size_t{ 0 };

		for (const auto& f : filters) {
			to_reserve += f.description.length();
			to_reserve += f.extension.length() + 1;
			to_reserve += 2;
		}

		filter.reserve(to_reserve);

		for (const auto& f : filters) {
			const auto description = widen(f.description);
			const auto extension = widen(f.extension);

			filter += widen(f.description);
			filter.push_back(L'\0');
			filter += L'*';
			filter += widen(f.extension);
			filter.push_back(L'\0');
		}

		return filter;
	}

	std::optional<std::string> window::open_file_dialog(
		const std::vector<file_dialog_filter>& filters,
		const std::string& custom_title
	) const {
		const auto filter = get_filter(filters);
		const auto title = widen(custom_title);
		
		OPENFILENAME ofn;       // common dialog box structure
		std::array<wchar_t, 400> szFile;
		fill_range(szFile, 0);

		ZeroMemory(&ofn, sizeof(ofn));
		ofn.lStructSize = sizeof(ofn);
		ofn.lpstrFile = szFile.data();
		ofn.hwndOwner = hwnd;
		ofn.lpstrFile[0] = '\0';
		ofn.nMaxFile = static_cast<DWORD>(szFile.size());
		ofn.lpstrFilter = filter.data();
		ofn.lpstrTitle = title.data();
		ofn.nFilterIndex = 1;
		ofn.lpstrFileTitle = NULL;
		ofn.nMaxFileTitle = 0;
		ofn.lpstrInitialDir = NULL;
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_EXPLORER | OFN_NOCHANGEDIR;

		// Display the Open dialog box. 

		if (GetOpenFileName(&ofn) == TRUE) {
			auto result = augs::path_type(ofn.lpstrFile);
			return str_ops(result.string()).replace_all("\\", "/");
		}
		else {
			return std::nullopt;
		}
	}

	std::optional<std::string> window::choose_directory_dialog(
		const std::string& custom_title
	) const {
		ensure(false && "This is not yet implemented.");
		return std::nullopt;
	}

	std::optional<std::string> window::save_file_dialog(
		const std::vector<file_dialog_filter>& filters,
		const std::string& custom_title
	) const {
		const auto filter = get_filter(filters);
		const auto title = widen(custom_title);

		OPENFILENAME ofn;       // common dialog box structure
		std::array<wchar_t, 400> szFile;
		fill_range(szFile, 0);

		ZeroMemory(&ofn, sizeof(ofn));
		ofn.lStructSize = sizeof(ofn);
		ofn.lpstrFile = szFile.data();
		ofn.hwndOwner = hwnd;
		ofn.lpstrFile[0] = '\0';
		ofn.nMaxFile = static_cast<DWORD>(szFile.size());
		ofn.lpstrFilter = filter.data();
		ofn.lpstrTitle = title.data();
		ofn.nFilterIndex = 1;
		ofn.lpstrFileTitle = NULL;
		ofn.nMaxFileTitle = 0;
		ofn.lpstrInitialDir = NULL;
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_EXPLORER | OFN_NOCHANGEDIR | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;

		// Display the Open dialog box. 

		if (GetSaveFileName(&ofn) == TRUE) {
			auto result = augs::path_type(ofn.lpstrFile);
			const auto supposed_extension = filters[ofn.nFilterIndex - 1].extension;

			if (supposed_extension != ".*") {
				if (result.extension() != supposed_extension) {
					result += filters[ofn.nFilterIndex - 1].extension;
				}
			}

			return str_ops(result.string()).replace_all("\\", "/");
		}
		else {
			return std::nullopt;
		}
	}

	void window::reveal_in_explorer(const augs::path_type& p) const {
		/*
			Could be implemented as:
			augs::shell(p.string());
			At least for directories. What about files?
		*/
	}

	void window::set_cursor_pos(vec2i pos) {
		pos += get_window_rect().get_position();

		SetCursorPos(pos.x, pos.y);
	}

	void window::clip_system_cursor() {
		thread_local RECT r;
		
		const ltrbi lt = get_window_rect();

		r.bottom = lt.b;
		r.left = lt.l;
		r.right = lt.r;
		r.top = lt.t;

		ClipCursor(&r);
	}

	void window::disable_cursor_clipping() {
		ClipCursor(NULL);
	}

	void window::set_cursor_visible(const bool flag) {
		if (!flag) {
			while (ShowCursor(FALSE) >= 0);
		}
		else {
			while (ShowCursor(TRUE) <= 0);
		}
	}
}

