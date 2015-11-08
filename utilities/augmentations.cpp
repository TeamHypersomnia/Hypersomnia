#include <Windows.h>
#include <gdiplus.h>

#include <GL/OpenGL.h>

#include <freetype\ft2build.h>
#include FT_FREETYPE_H

#include "augmentations.h"
#include "options.h"
#include "window_framework\window.h"

namespace augs {
	Gdiplus::GdiplusStartupInput gdi;
	ULONG_PTR           gdit;
	WNDCLASSEX wcl = {0};
	HINSTANCE hinst;
	unsigned initialized = 0;
	std::unique_ptr<FT_Library> freetype_library(new FT_Library);

	namespace window {
		extern LRESULT CALLBACK wndproc(HWND, UINT, WPARAM, LPARAM);
	};

	void init(unsigned to_initialize) {
		if(to_initialize & GDIPLUS) 
			errs(Gdiplus::GdiplusStartup(&gdit, &gdi, nullptr) == Gdiplus::Status::Ok, L"Failed to initialize GDI+!");
		if(to_initialize & FREETYPE)
			errs(!FT_Init_FreeType(freetype_library.get()), "freetype initialization");
		if(to_initialize & WINDOWS_API) {
			hinst = GetModuleHandle(NULL);
			wcl.cbSize = sizeof(wcl);
			wcl.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
			wcl.lpfnWndProc = window::wndproc;
			wcl.cbClsExtra = 0;
			wcl.cbWndExtra = 0;
			wcl.hInstance = GetModuleHandle(NULL);
			wcl.hIcon = LoadIcon(0, IDI_APPLICATION);
			wcl.hCursor = LoadCursor(0, IDC_ARROW);
			wcl.hbrBackground = 0;
			wcl.lpszMenuName = 0;
			wcl.lpszClassName = L"AugmentedWindow";
			wcl.hIconSm = 0;

			(errs((RegisterClassEx(&wcl) != 0), "class registering") != 0);
		}
		
		if(to_initialize & GLEW) {
			window::glwindow dummy;
			dummy.create(rects::xywh<int>(10, 10, 200, 200));
			
			glewExperimental = FALSE;
			errsl((error_logging::glew_last_errorcode = glewInit()) == GLEW_OK, glew_errors, L"Failed to initialize GLEW");
		}

		initialized |= to_initialize;
	}

	void deinit() {
		if(initialized & GDIPLUS) 
			Gdiplus::GdiplusShutdown(gdit);
		if(initialized & GLEW) {
		
		}
		if(initialized & FREETYPE) 
			errs(!FT_Done_FreeType(*freetype_library.get()), "freetype deinitialization");
	}
};