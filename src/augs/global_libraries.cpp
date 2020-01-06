#if BUILD_NETWORKING
#include "augs/network/network_types.h"
#endif

#if BUILD_FREETYPE
#include <ft2build.h>
#include FT_FREETYPE_H
#endif

#undef min
#undef max

#include "augs/log.h"
#include "augs/ensure.h"
#include "augs/global_libraries.h"

#if BUILD_FREETYPE
std::unique_ptr<FT_Library> augs::freetype_raii::freetype_library(std::make_unique<FT_Library>());
#endif

namespace augs {
#if BUILD_FREETYPE
	freetype_raii::freetype_raii() {
		const auto success = !FT_Init_FreeType(freetype_library.get()) && "freetype initialization";
		(void)success;
		ensure(success);
	}

	freetype_raii::~freetype_raii() {
		const auto success = !FT_Done_FreeType(*freetype_library.get()) && "freetype deinitialization";
		(void)success;
		ensure(success);
	}
#endif

#if BUILD_NETWORKING
	network_raii::network_raii() {
		network::init();
	}

	network_raii::~network_raii() {
		network::deinit();
	}
#endif
};
