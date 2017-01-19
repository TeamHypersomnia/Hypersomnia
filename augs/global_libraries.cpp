#include <enet/enet.h>
#undef min
#undef max

#ifdef PLATFORM_WINDOWS
#include <Windows.h>
#endif

#ifdef PLATFORM_WINDOWS
#include <GL/OpenGL.h>
#elif PLATFORM_LINUX
#include <GL/gl.h>
#endif

#ifdef PLATFORM_WINDOWS
#include <ft2build.h>
#elif PLATFORM_LINUX
#include <ft2build.h>
#endif
#include FT_FREETYPE_H

#include "global_libraries.h"
#include "augs/window_framework/window.h"
#include "augs/window_framework/platform_utils.h"

#include <gtest/gtest.h>
#include "error/augs_error.h"
#include "augs/log.h"
#include "augs/ensure.h"

#include <signal.h>


void SignalHandler(int signal) {
	augs::window::disable_cursor_clipping();
	throw "Access violation!";
}

namespace augs {
	namespace window {
#ifdef PLATFORM_WINDOWS
    extern LRESULT CALLBACK wndproc(HWND, UINT, WPARAM, LPARAM);
#endif
  };

	unsigned global_libraries::initialized = 0;
	std::unique_ptr<FT_Library> global_libraries::freetype_library(new FT_Library);

	void global_libraries::init(const unsigned to_initialize) {
		// signal(SIGSEGV, SignalHandler);

		if(to_initialize & FREETYPE)
			ensure(!FT_Init_FreeType(freetype_library.get()) && "freetype initialization");
#ifdef PLATFORM_WINDOWS
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

			ensure(RegisterClassEx(&wcl) != 0 && "class registering");
		}

		
		if(to_initialize & GLEW) {
			window::glwindow dummy;
			dummy.create(xywhi(10, 10, 200, 200));
			
			glewExperimental = FALSE;
			ensure(glewInit() == GLEW_OK && L"Failed to initialize GLEW");
		}

		if (to_initialize & ENET) {
			ensure(enet_initialize() == 0 && L"Failed to initialize enet");
		}
#endif
		initialized |= to_initialize;
	}

	void global_libraries::deinit(const unsigned which_augs) {
		if (which_augs & GLEW) {

		}

		if (which_augs & FREETYPE) {
			ensure(initialized & FREETYPE);
			ensure(!FT_Done_FreeType(*freetype_library.get()) && "freetype deinitialization");
			initialized &= ~FREETYPE;
		}

		if (which_augs & ENET) {
			ensure(initialized & ENET);
			enet_deinitialize();
			initialized &= ~ENET;
		}
	}

	void global_libraries::run_googletest(int argc, char** argv) {
		::testing::InitGoogleTest(&argc, argv);
		auto result = RUN_ALL_TESTS();
	}
};
