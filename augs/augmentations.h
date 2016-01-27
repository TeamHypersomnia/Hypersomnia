#pragma once

#include <memory>

struct FT_LibraryRec_;
typedef struct FT_LibraryRec_  *FT_Library;

namespace augs {
	enum which_augs {
		GDIPLUS = 1<<0,
		GLEW = 1<<1,
		FREETYPE = 1<<2,
		WINDOWS_API = 1<<3,
		ALL = GDIPLUS | GLEW | FREETYPE | WINDOWS_API 
	};
	
	extern unsigned initialized;

	extern std::unique_ptr<FT_Library> freetype_library;
	
	extern void run_tests();
	extern void init(unsigned which_augs = ALL);
	extern void deinit();
};