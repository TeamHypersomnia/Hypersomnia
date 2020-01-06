#pragma once
#include <memory>

#if BUILD_FREETYPE
struct FT_LibraryRec_;
typedef struct FT_LibraryRec_  *FT_Library;
#endif

namespace augs {
	struct freetype_raii {
#if BUILD_FREETYPE
		static std::unique_ptr<FT_Library> freetype_library;

		freetype_raii();
		~freetype_raii();
#endif
	};

	struct network_raii {
		network_raii();
		~network_raii();

		network_raii(const network_raii&) = delete;
		network_raii(network_raii&&) = delete;

		network_raii& operator=(const network_raii&) = delete;
		network_raii& operator=(network_raii&&) = delete;
	};
};