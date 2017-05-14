#pragma once
#include <memory>
#include "augs/misc/enum_boolset.h"

struct FT_LibraryRec_;
typedef struct FT_LibraryRec_  *FT_Library;

namespace augs {
	struct global_libraries {
		enum class library {
			FREETYPE,
			ENET,

			COUNT
		};

		typedef augs::enum_boolset<library> library_flagset;

		static library_flagset initialized;

		static std::unique_ptr<FT_Library> freetype_library;

		static void run_unit_tests(
			const int argc, 
			const char* const * const argv,
			const bool show_successful,
			const bool break_on_failure
		);

		static void init  (const library_flagset = { library::FREETYPE, library::ENET });
		static void deinit(const library_flagset = { library::FREETYPE, library::ENET });
	};
};