#include "augs/window_framework/platform_utils.h"
#include "augs/templates/string_templates.h"

namespace augs {
	bool is_character_newline(const unsigned i) {
		return (i == 0x000A || i == 0x000D);
	}
}

#if BUILD_WINDOW_FRAMEWORK

#if PLATFORM_WINDOWS
#include <Windows.h>
#undef min
#undef max

namespace augs {
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

	void set_cursor_pos(const vec2i pos) {
		SetCursorPos(pos.x, pos.y);
	}

	std::optional<vec2i> get_cursor_pos() {
		POINT p;

		if (GetCursorPos(&p)) {
			return vec2i { p.x, p.y };
		}

		return std::nullopt;
	}
}

#elif PLATFORM_UNIX

namespace augs {
	bool set_display(const vec2i v, const int bpp) {
		return true;
	}

	xywhi get_display() {
		return {};
	}

	void set_cursor_visible(const bool flag) {

	}

	void set_cursor_pos(const vec2i pos) {
	
	}

	std::optional<vec2i> get_cursor_pos() {
		return std::nullopt;
	}
}

#else
#error "Unsupported platform!"
#endif

#else
namespace augs {
	bool set_display(const vec2i v, const int bpp) {
		return true;
	}

	xywhi get_display() {
		return {};
	}

	void set_cursor_visible(const bool flag) {

	}

	void set_cursor_pos(const vec2i pos) {

	}

	std::optional<vec2i> get_cursor_pos() {
		return std::nullopt;
	}
}

#endif
