#include <signal.h>

#if BUILD_NETWORKING

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
#include "augs/window_framework/platform_utils.h"

#if BUILD_FREETYPE
std::unique_ptr<FT_Library> augs::global_libraries::freetype_library(new FT_Library);
#endif

namespace augs {
	global_libraries::global_libraries(const library_flagset to_initialize) {
		init(to_initialize);
	}

	void global_libraries::init(const library_flagset to_initialize) {
		if(to_initialize.test(library::FREETYPE)) {
#if BUILD_FREETYPE
			const auto success = !FT_Init_FreeType(freetype_library.get()) && "freetype initialization";
			ensure(success);
			initialized.set(library::FREETYPE);
#endif
		}
		
		if(to_initialize.test(library::NETWORKING)) {
#if BUILD_NETWORKING
			const auto success = true;
			ensure(success && "Failed to initialize networking");
			initialized.set(library::NETWORKING);
#endif
		}
	}

	void global_libraries::deinit(const library_flagset to_deinitialize) {
		if(to_deinitialize.test(library::FREETYPE)) {
#if BUILD_FREETYPE
			const auto success = !FT_Done_FreeType(*freetype_library.get()) && "freetype deinitialization";
			ensure(initialized.test(library::FREETYPE));
			ensure(success);
			initialized.set(library::FREETYPE, false);
#endif
		}

		if(to_deinitialize.test(library::NETWORKING)) {
#if BUILD_NETWORKING
			ensure(initialized.test(library::NETWORKING));
			// false
			initialized.set(library::NETWORKING, false);
#endif
		}
	}

	global_libraries::~global_libraries() {
		deinit();
	}
};
