#pragma once
#include <memory>

struct FT_LibraryRec_;
typedef struct FT_LibraryRec_  *FT_Library;

namespace augs {
	struct global_libraries {
		enum library {
			GLEW = 1 << 1,
			FREETYPE = 1 << 2,
			WINDOWS_API = 1 << 3,
			ALL = GLEW | FREETYPE | WINDOWS_API
		};

		static unsigned initialized;

		static std::unique_ptr<FT_Library> freetype_library;

		static void run_googletest(int argc, char** argv);

		static void init(unsigned which_augs = ALL);
		static void deinit(unsigned which_augs = ALL);
	};

};