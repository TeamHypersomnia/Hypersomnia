#include "platform_utils.h"
#ifdef PLATFORM_WINDOWS
#include <Windows.h>
#elif PLATFORM_LINUX
#endif
#include "augs/templates/string_templates.h"

#ifdef PLATFORM_WINDOWS
namespace augs {
	/*
	void set_clipboard_data(std::string from) {
		if (OpenClipboard(0)) {
			if (EmptyClipboard()) {
				HGLOBAL h = GlobalAlloc(GMEM_DDESHARE, (from.length() + 1) * sizeof(WCHAR));
				if (h) {
					LPWSTR p = (LPWSTR)GlobalLock(h);

					if (p) {
						for (unsigned i = 0; i < from.length(); ++i)
							p[i] = from[i];

						p[from.length()] = 0;

						SetClipboardData(CF_UNICODETEXT, h);
						GlobalUnlock(p);
						CloseClipboard();
					}
				}
			}
		}
	}
	*/
	bool is_character_newline(unsigned i) {
		return (i == 0x000A || i == 0x000D);
	}
	/*
	std::string get_data_from_clipboard() {
		std::string result;

		if (OpenClipboard(NULL)) {
			if (!IsClipboardFormatAvailable(CF_UNICODETEXT)) {
				CloseClipboard();
				return result;
			}
			HANDLE clip0 = GetClipboardData(CF_UNICODETEXT);

			if (clip0) {
				LPWSTR p = (LPWSTR)GlobalLock(clip0);
				if (p) {
					size_t len = wcslen(p);
					result.clear();
					result.reserve(len);

					for (size_t i = 0; i < len; ++i) {
						result += p[i];
						if (is_character_newline(p[i]) && i < len - 1 && is_character_newline(p[i + 1])) ++i;
					}
				}
				GlobalUnlock(clip0);
			}

			CloseClipboard();
		}

		return result;
	}*/

	void enable_cursor_clipping(const ltrbi lt) {
		thread_local RECT r;
		r.bottom = lt.b;
		r.left = lt.l;
		r.right = lt.r;
		r.top = lt.t;
		ClipCursor(&r);
	}

	void disable_cursor_clipping() {
		ClipCursor(NULL);
	}


	bool set_display(const vec2i v, const int bpp) {
		static DEVMODE screen;
		ZeroMemory(&screen, sizeof(screen));
		screen.dmSize = sizeof(screen);
		screen.dmPelsWidth = v.x;
		screen.dmPelsHeight = v.y;
		screen.dmBitsPerPel = bpp;
		screen.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;
		return ChangeDisplaySettings(&screen, CDS_FULLSCREEN) == DISP_CHANGE_SUCCESSFUL;
	}

	xywhi get_display() {
		static RECT rc;
		GetWindowRect(GetDesktopWindow(), &rc);
		return xywhi(rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top);
	}

	void set_cursor_visible(const bool flag) {
		if (!flag) {
			while (ShowCursor(FALSE) >= 0);
		}
		else {
			while (ShowCursor(TRUE) <= 0);
		}
	}
}
#elif PLATFORM_LINUX
#endif
