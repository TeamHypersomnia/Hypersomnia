#include "augs/window_framework/platform_utils.h"
#include "augs/templates/string_templates.h"

namespace augs {
	bool is_character_newline(const unsigned i) {
		return (i == 0x000A || i == 0x000D);
	}
}

#if PLATFORM_WINDOWS
#include <Windows.h>

namespace augs {
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
#else

namespace augs {
	void enable_cursor_clipping(const ltrbi lt) {
		
	}

	void disable_cursor_clipping() {

	}

	bool set_display(const vec2i v, const int bpp) {
		return true;
	}

	xywhi get_display() {
		return {};
	}

	void set_cursor_visible(const bool flag) {

	}
}

#endif
