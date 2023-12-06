#include "augs/window_framework/platform_utils.h"
#include "augs/string/string_templates.h"

#include "augs/filesystem/file.h"
#include "augs/window_framework/shell.h"
#include "augs/window_framework/exec.h"

namespace augs {
	bool is_character_newline(const unsigned i) {
		return (i == 0x000A || i == 0x000D);
	}
}

#if BUILD_WINDOW_FRAMEWORK

#if USE_GLFW
namespace augs {
	bool set_display(const vec2i, const int) {
		return true;
	}

	xywhi get_display() {
		return {};
	}

	std::optional<vec2i> find_cursor_pos() {
		return std::nullopt;
	}
}

#elif PLATFORM_WINDOWS
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

	xywhi get_display_no_window() {
		static RECT rc;
		GetWindowRect(GetDesktopWindow(), &rc);
		return xywhi(rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top);
	}

	std::optional<vec2i> find_cursor_pos() {
		POINT p;

		if (GetCursorPos(&p)) {
			return vec2i { p.x, p.y };
		}

		return std::nullopt;
	}
}

#elif PLATFORM_UNIX
#include "augs/misc/scope_guard.h"

#include <X11/Xlib.h>

namespace augs {
	bool set_display(const vec2i /* v */, const int /* bpp */) {
		return true;
	}

	xywhi get_display_no_window() {
		if (Display* d = XOpenDisplay(NULL)) {
			auto guard = augs::scope_guard([d](){ XCloseDisplay(d); });

			if (Screen*  s = DefaultScreenOfDisplay(d)) {
				return { 0, 0, s->width, s->height };
			}
		}

		return {};
	}

	std::optional<vec2i> find_cursor_pos() {
		return std::nullopt;
	}

}

#else
#error "Unsupported platform!"
#endif

#else
namespace augs {
	bool set_display(const vec2i, const int) {
		return true;
	}

	xywhi get_display() {
		return {};
	}

	std::optional<vec2i> find_cursor_pos() {
		return std::nullopt;
	}
}

#endif

namespace augs {
#ifdef __APPLE__   
#include "CoreFoundation/CoreFoundation.h"
#include <unistd.h>
#include <libgen.h>
	path_type get_executable_path() {
		CFBundleRef mainBundle = CFBundleGetMainBundle();
		CFURLRef exeURL = CFBundleCopyExecutableURL(mainBundle);
		char path[PATH_MAX];
		if (!CFURLGetFileSystemRepresentation(exeURL, TRUE, (UInt8 *)path, PATH_MAX))
		{
			// error!
		}
		CFRelease(exeURL);

		return path;
	}

#elif PLATFORM_UNIX
#include <unistd.h>
	path_type get_executable_path() {
		char dest[PATH_MAX];
		memset(dest,0,sizeof(dest)); // readlink does not null terminate!

		if (readlink("/proc/self/exe", dest, PATH_MAX) == -1) {

		} 
		else {
			return dest;
		}

		return augs::path_type();
	}
#endif
}

#if PLATFORM_WINDOWS
#include <Shlobj.h>

namespace augs {
	path_type get_default_documents_dir() {
		char path[MAX_PATH];
		HRESULT result = SHGetFolderPathA(NULL, CSIDL_MYDOCUMENTS, NULL, SHGFP_TYPE_CURRENT, path);

		if (SUCCEEDED(result)) {
			return path_type(path) / "My Games" / "Hypersomnia";
		}
		else {
			// Handle error
			return path_type();
		}
	}
}
#else
namespace augs {
	path_type get_default_documents_dir() {
		/* Default to CWD */
		return "";
	}
}
#endif