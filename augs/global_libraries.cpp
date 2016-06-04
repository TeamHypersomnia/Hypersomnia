#include <Windows.h>

#include <GL/OpenGL.h>

#include <freetype\ft2build.h>
#include FT_FREETYPE_H

#include "global_libraries.h"
#include "window_framework\window.h"

#include <gtest/gtest.h>
#include "error/augs_error.h"
#include "log.h"
#include "ensure.h"

namespace augs {
	namespace window {
		extern LRESULT CALLBACK wndproc(HWND, UINT, WPARAM, LPARAM);
	};

	unsigned global_libraries::initialized = 0;
	std::unique_ptr<FT_Library> global_libraries::freetype_library(new FT_Library);

	void global_libraries::init(unsigned to_initialize) {
		if(to_initialize & FREETYPE)
			errs(!FT_Init_FreeType(freetype_library.get()), "freetype initialization");
		if(to_initialize & WINDOWS_API) {
			WNDCLASSEX wcl = { 0 };
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
			errs(glewInit() == GLEW_OK, L"Failed to initialize GLEW");
		}

		initialized |= to_initialize;
	}

	void global_libraries::deinit(unsigned which_augs) {
		if (which_augs & GLEW) {

		}
		if (which_augs & FREETYPE) {
			ensure(initialized & FREETYPE);
			errs(!FT_Done_FreeType(*freetype_library.get()), "freetype deinitialization");
			initialized &= ~FREETYPE;
		}
	}

	void global_libraries::run_googletest() {
		int argc = 0;
		::testing::InitGoogleTest(&argc, (wchar_t**)nullptr);

		::testing::FLAGS_gtest_catch_exceptions = false;
		::testing::FLAGS_gtest_break_on_failure = false;
		auto result = RUN_ALL_TESTS();
	}
};