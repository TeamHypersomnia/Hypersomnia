#include "platform_utils.h"
#include <Windows.h>
#include <Shlwapi.h>

namespace augs {
	namespace window {
		void set_clipboard_data(std::wstring from) {
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

		bool is_character_newline(unsigned i) {
			return (i == 0x000A || i == 0x000D);
		}

		std::wstring get_data_from_clipboard() {
			std::wstring result;

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
		}

		std::wstring get_executable_path() {
			wchar_t buffer[MAX_PATH + 1];
			SecureZeroMemory(buffer, sizeof(buffer));
			GetModuleFileName(NULL, buffer, MAX_PATH);
			PathRemoveFileSpec(buffer);
			return buffer;
		}

		void enable_cursor_clipping(rects::ltrb<int> lt) {
			static thread_local RECT r;
			r.bottom = lt.b;
			r.left = lt.l;
			r.right = lt.r;
			r.top = lt.t;
			ClipCursor(&r);
		}

		void disable_cursor_clipping() {
			ClipCursor(NULL);
		}


		bool set_display(int width, int height, int bpp) {
			static DEVMODE screen;
			ZeroMemory(&screen, sizeof(screen));
			screen.dmSize = sizeof(screen);
			screen.dmPelsWidth = width;
			screen.dmPelsHeight = height;
			screen.dmBitsPerPel = bpp;
			screen.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;
			return ChangeDisplaySettings(&screen, CDS_FULLSCREEN) == DISP_CHANGE_SUCCESSFUL;
		}

		rects::xywh<int> get_display() {
			static RECT rc;
			GetWindowRect(GetDesktopWindow(), &rc);
			return rects::xywh<int>(rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top);
		}

		void set_cursor_visible(int flag) {
			ShowCursor(flag);
		}
	}
}