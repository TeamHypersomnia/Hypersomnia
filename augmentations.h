#pragma once

/*
Debug:

glew32sd.lib
OpenGL32.lib
Ws2_32.lib
dwmapi.lib
freetype2412MT_D.lib
Gdiplus.lib

Release:

glew32s.lib
OpenGL32.lib
Ws2_32.lib
dwmapi.lib
freetype2412MT.lib
Gdiplus.lib
*/

#include <memory>

struct FT_LibraryRec_;
typedef struct FT_LibraryRec_  *FT_Library;

namespace augmentations {
	enum which_augmentations {
		GDIPLUS = 1<<0,
		GLEW = 1<<1,
		FREETYPE = 1<<2,
		WINDOWS_API = 1<<3,
		ALL = GDIPLUS | GLEW | FREETYPE | WINDOWS_API 
	};
	
	extern unsigned initialized;

	extern std::unique_ptr<FT_Library> freetype_library;

	extern void init(unsigned which_augmentations = ALL);
	extern void deinit();
};