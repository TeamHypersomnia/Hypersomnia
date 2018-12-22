#include <algorithm>

#include "augs/log.h"

#include "augs/string/string_templates.h"
#include "augs/templates/corresponding_field.h"
#include "augs/templates/algorithm_templates.h"

#include "augs/window_framework/window.h"
#include "augs/window_framework/platform_utils.h"

#include <Windows.h>
#include <shlobj.h>
#include "augs/graphics/OpenGL_includes.h"

#include "3rdparty/glad/glad_wgl.h"
#include "3rdparty/glad/glad_wgl.c"

#include "augs/window_framework/translate_winapi_enums.h"

static std::string PickContainer(const std::wstring& custom_title) {
	std::string result;
    IFileDialog *pfd;
    
    if (SUCCEEDED(CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pfd))))
    {
        DWORD dwOptions;
        if (SUCCEEDED(pfd->GetOptions(&dwOptions)))
        {
            pfd->SetOptions(dwOptions | FOS_PICKFOLDERS);
        }
        
        pfd->SetTitle(custom_title.c_str());

        if (SUCCEEDED(pfd->Show(NULL)))
        {
            IShellItem *psi;
            if (SUCCEEDED(pfd->GetResult(&psi)))
            {
				LPWSTR g_path = nullptr;
                if(!SUCCEEDED(psi->GetDisplayName(SIGDN_DESKTOPABSOLUTEPARSING, &g_path)))
                {
                    MessageBox(NULL, L"GetIDListName() failed", NULL, NULL);
                }

			result = augs::path_type(g_path).string();
			str_ops(result).replace_all("\\", "/");

                psi->Release();
            }
        }
        pfd->Release();
    }

    return result;
}

namespace augs {
	struct window::platform_data {
		HWND hwnd = nullptr;
		HDC hdc = nullptr;
		HGLRC hglrc = nullptr;

		vec2i min_window_size;
		vec2i max_window_size;

		int style = 0xdeadbeef;
		int exstyle = 0xdeadbeef;

		bool double_click_occured = false;

		timer triple_click_timer;
		unsigned triple_click_delay = 0xdeadbeef;
	};

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

	template <class U, class W, class L>
	std::optional<event::change> window::handle_event(const U m, const W wParam, const L lParam) {
		using namespace event::keys;

		event::change change;
		change.msg = translate_enum(m);

		auto default_proc = [&]() {
			DefWindowProc(platform->hwnd, m, wParam, lParam);
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

			if (current_settings.log_keystrokes) {
				LOG("Keyup: %x", key_to_string(change.data.key.key));
			}

			return change;
		case WM_KEYDOWN:
			change.data.key.key = translate_key_with_lparam(lParam, wParam);

			if (/* repeated */ ((lParam & (1 << 30)) != 0)) {
				return std::nullopt;
			}

			if (current_settings.log_keystrokes) {
				LOG("Keydown: %x", key_to_string(change.data.key.key));
			}

			return change;

		case WM_SYSKEYUP:
			change.data.key.key = translate_key_with_lparam(lParam, wParam);

			if (current_settings.log_keystrokes) {
				LOG("Syskeyup: %x", key_to_string(change.data.key.key));
			}

			return change;
		case WM_SYSKEYDOWN:
			change.data.key.key = translate_key_with_lparam(lParam, wParam);

			if (/* repeated */ ((lParam & (1 << 30)) != 0)) {
				return std::nullopt;
			}

			if (current_settings.log_keystrokes) {
				LOG("Syskeydown: %x", key_to_string(change.data.key.key));
			}

			return change;

		case WM_MOUSEWHEEL:
			change.data.scroll.amount = GET_WHEEL_DELTA_WPARAM(wParam) / WHEEL_DELTA;
			return change;
		
		// Doubleclicks
		
		case WM_LBUTTONDBLCLK:
			SetCapture(platform->hwnd);
			
			platform->triple_click_timer.extract<std::chrono::microseconds>();
			platform->double_click_occured = true;

			return change;

		case WM_RBUTTONDBLCLK:
		case WM_MBUTTONDBLCLK:
			return change;

		// Downs
		
		case WM_LBUTTONDOWN:
			change.data.key.key = key::LMOUSE;

			SetCapture(platform->hwnd);

			if (platform->double_click_occured && platform->triple_click_timer.extract<std::chrono::milliseconds>() < platform->triple_click_delay) {
				change.msg = event::message::ltripleclick;
				platform->double_click_occured = false;
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
			if (GetCapture() == platform->hwnd) ReleaseCapture(); return change;
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

				platform->double_click_occured = false;

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
				mi->ptMinTrackSize.x = platform->min_window_size.x;
				mi->ptMinTrackSize.y = platform->min_window_size.y;
				mi->ptMaxTrackSize.x = platform->max_window_size.x;
				mi->ptMaxTrackSize.y = platform->max_window_size.y;
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
		SetWindowText(platform->hwnd, widen(name).c_str());
	}

	window::window(
		const window_settings& settings
	) : platform(std::make_unique<window::platform_data>()) {
		// TODO: throw an exception instead of ensuring
		static bool register_once = [](){
			LOG("WINAPI: Registering the window class.");
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

			LOG("WINAPI: Calling RegisterClassEx.");
			const auto register_success = RegisterClassEx(&wcl) != 0;

			ensure(register_success && "class registering");
			return register_success;
		}();
		(void)register_once;

		LOG("WINAPI: Calling GetDoubleClickTime.");
		platform->triple_click_delay = GetDoubleClickTime();

		auto make_window = [&]() {
			LOG("WINAPI: Calling CreateWindowEx.");
			platform->hwnd = CreateWindowEx(0, L"augwin", L"invalid_name", 0, 0, 0, 0, 0, 0, 0, GetModuleHandle(NULL), this);
			ensure(platform->hwnd);

			LOG("WINAPI: Calling GetDC.");
			platform->hdc = GetDC(platform->hwnd);
			ensure(platform->hdc);
		};

		auto make_context = [&]() {
#if BUILD_OPENGL
			platform->hglrc = wglCreateContext(platform->hdc);
			ensure(platform->hglrc);
#endif
			const auto sc = set_as_current();
			(void)sc;
			ensure(sc);
		};

		make_window();

		LOG("WINAPI: Filling PIXELFORMATDESCRIPTOR.");
		PIXELFORMATDESCRIPTOR p;
		ZeroMemory(&p, sizeof(p));

		p.nSize = sizeof(p);
		p.nVersion = 1;
		p.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
		p.iPixelType = PFD_TYPE_RGBA;
		p.cColorBits = settings.bpp;
		p.cAlphaBits = 8;
		p.cStencilBits = 8;
		p.cDepthBits = 0;
		p.cAuxBuffers = 0;
		p.cAccumBits = 0;
		p.iLayerType = PFD_MAIN_PLANE;

		{
			LOG("WINAPI: Calling ChoosePixelFormat.");
			const auto pf = ChoosePixelFormat(platform->hdc, &p);
			(void)pf;
			ensure(pf);

			LOG("WINAPI: Calling SetPixelFormat.");
			const auto result = SetPixelFormat(platform->hdc, pf, &p);
			(void)result;
			ensure(result);
		}


		{
			LOG("Making dummy context");
			make_context();

			LOG("WINAPI: Calling gladLoadWGL.");
			if (!gladLoadWGL(platform->hdc)) {
				throw window_error("Failed to initialize GLAD WGL!"); 		
			}

			set_current_to_none();
			destroy();
		
			make_window();

			const int attribList[] = {
    			WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
    			WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
    			WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
    			WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
    			WGL_COLOR_BITS_ARB, static_cast<int>(settings.bpp),
    			WGL_ALPHA_BITS_ARB, 8,
    			WGL_DEPTH_BITS_ARB, 0,
    			WGL_STENCIL_BITS_ARB, 8,
    			WGL_SAMPLE_BUFFERS_ARB, 0,
    			WGL_SAMPLES_ARB, 0,
    			WGL_AUX_BUFFERS_ARB, 0,
    			WGL_ACCUM_BITS_ARB, 0,
    			WGL_ACCUM_RED_BITS_ARB, 0,
    			WGL_ACCUM_GREEN_BITS_ARB, 0,
    			WGL_ACCUM_BLUE_BITS_ARB, 0,
    			WGL_ACCUM_ALPHA_BITS_ARB, 0,
    			0,
			};

			int pixelFormat;
			UINT numFormats;

			{
				LOG("WINAPI: Calling wglChoosePixelFormatARB.");
				const auto result = wglChoosePixelFormatARB(platform->hdc, attribList, NULL, 1, &pixelFormat, &numFormats);
				(void)result;
				ensure(result);
			}

			LOG_NVPS(pixelFormat, numFormats);
			
			{
				LOG("WINAPI: Calling SetPixelFormat.");
				const auto result = SetPixelFormat(platform->hdc, pixelFormat, &p);
				(void)result;
				ensure(result);
			}
			
			LOG("Making context proper");

			make_context();
		}

		show();

		LOG("WINAPI: Calling SetLastError.");
		SetLastError(0);

		{
			LOG("WINAPI: Calling SetWindowLongPtr.");

			const auto result = !(SetWindowLongPtr(platform->hwnd, GWLP_USERDATA, (LONG_PTR)this) == 0 && GetLastError() != 0);
			(void)result;
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
		Rid[0].hwndTarget = platform->hwnd;
		LOG("WINAPI: Calling RegisterRawInputDevices.");
		RegisterRawInputDevices(Rid, 1, sizeof(Rid[0]));

		apply(settings, true);
		last_mouse_pos = get_screen_size() / 2;

		const auto app_icon_path = settings.app_icon_path.wstring();
		
		if (/* icon_was_specified */ !app_icon_path.empty()) {
			LOG("WINAPI: Calling LoadImage.");

			const auto app_icon = (HICON)LoadImage(NULL,
				app_icon_path.c_str(),
				IMAGE_ICON,       
				0,                
				0,                
				LR_LOADFROMFILE |  
				LR_DEFAULTSIZE |   
				LR_SHARED         
			);

			LOG("WINAPI: Calling SendMessage.");
			SendMessage(platform->hwnd, WM_SETICON, ICON_SMALL, (LPARAM)app_icon);
			SendMessage(platform->hwnd, WM_SETICON, ICON_BIG, (LPARAM)app_icon);
		}
	}

	void window::destroy() {
		if (platform->hwnd) {
			unset_if_current();

#if BUILD_OPENGL
			LOG("WINAPI: Calling wglDeleteContext.");
			wglDeleteContext(platform->hglrc);
#endif
			LOG("WINAPI: Calling ReleaseDC.");
			ReleaseDC(platform->hwnd, platform->hdc);

			LOG("WINAPI: Calling DestroyWindow.");
			DestroyWindow(platform->hwnd);

			platform->hwnd = nullptr;
		}
	}

	void window::set_window_border_enabled(const bool enabled) {
		if (enabled) {
			platform->style = WS_OVERLAPPEDWINDOW;
			platform->exstyle = WS_EX_WINDOWEDGE;
		}
		else {
			platform->style = WS_POPUP;
			platform->exstyle = WS_EX_APPWINDOW;
		}

		SetWindowLongPtr(platform->hwnd, GWL_EXSTYLE, platform->exstyle);
		SetWindowLongPtr(platform->hwnd, GWL_STYLE, platform->style);

		SetWindowPos(platform->hwnd, NULL, 0, 0, 0, 0, SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOZORDER | SWP_NOOWNERZORDER);
		set_window_rect(get_window_rect());
		show();
	}

	bool window::swap_buffers() {
		return SwapBuffers(platform->hdc) != FALSE;
	}

	void window::show() {
		LOG("WINAPI: Calling ShowWindow.");
		ShowWindow(platform->hwnd, SW_SHOW);
	}

	bool window::set_as_current_impl() {
#if BUILD_OPENGL
		return wglMakeCurrent(platform->hdc, platform->hglrc);
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

			while (PeekMessageW(&wmsg, platform->hwnd, 0, 0, PM_REMOVE)) {
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
			(AdjustWindowRectEx(&wr, platform->style, FALSE, platform->exstyle)) &&
			(MoveWindow(platform->hwnd, wr.left, wr.top, wr.right - wr.left, wr.bottom - wr.top, TRUE))
		;

		(void)result;
		ensure(result);
	}

	xywhi window::get_window_rect() const {
		static RECT r;
		GetClientRect(platform->hwnd, &r);
		ClientToScreen(platform->hwnd, (POINT*)&r);
		ClientToScreen(platform->hwnd, (POINT*)&r + 1);
		return ltrbi(r.left, r.top, r.right, r.bottom);
	}

	void window::set(const vsync_type mode) {
		switch (mode) {
			case vsync_type::OFF: wglSwapIntervalEXT(0); break;
			case vsync_type::ON: wglSwapIntervalEXT(1); break;
			case vsync_type::ADAPTIVE: wglSwapIntervalEXT(-1); break;

			default: wglSwapIntervalEXT(0); break;
		}
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
		ofn.hwndOwner = platform->hwnd;
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
		const auto title = widen(custom_title);
		const auto choice = ::PickContainer(title);

		if (choice.size() > 0) {
			return choice;
		}

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
		ofn.hwndOwner = platform->hwnd;
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

	window::~window() {
		destroy();
	}
}

