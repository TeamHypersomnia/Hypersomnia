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
			ENET = 1 << 4,
			ALL = GLEW | FREETYPE | WINDOWS_API | ENET
		};

		static unsigned initialized;

		static std::unique_ptr<FT_Library> freetype_library;

		static void run_unit_tests(
			const int argc, 
			const char* const * const argv,
			const bool show_successful,
			const bool break_on_failure
		);

		static void init(unsigned which_augs = ALL);
		static void deinit(unsigned which_augs = ALL);
	};
};