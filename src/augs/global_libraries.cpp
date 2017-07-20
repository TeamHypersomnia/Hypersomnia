#include <signal.h>
#if BUILD_ENET
#include <enet/enet.h>
#endif
#include <ft2build.h>
#include FT_FREETYPE_H

#include "global_libraries.h"
#include "augs/log.h"
#include "augs/ensure.h"
#include "augs/window_framework/platform_utils.h"

std::unique_ptr<FT_Library> augs::global_libraries::freetype_library(new FT_Library);

namespace augs {
	global_libraries::global_libraries(const library_flagset to_initialize) {
		init(to_initialize);
	}

	void global_libraries::init(const library_flagset to_initialize) {
#if 0
		signal(SIGSEGV, [](const int signal) {
			augs::disable_cursor_clipping();
			throw "Access violation!";
		});
#endif

		if(to_initialize.test(library::FREETYPE)) {
			ensure(!FT_Init_FreeType(freetype_library.get()) && "freetype initialization");
			initialized.set(library::FREETYPE);
		}
		
		if(to_initialize.test(library::ENET)) {
#if BUILD_ENET
			ensure(enet_initialize() == 0 && L"Failed to initialize enet");
			initialized.set(library::ENET);
#endif
		}
	}

	void global_libraries::deinit(const library_flagset to_deinitialize) {
		if(to_deinitialize.test(library::FREETYPE)) {
			ensure(initialized.test(library::FREETYPE));
			ensure(!FT_Done_FreeType(*freetype_library.get()) && "freetype deinitialization");
			initialized.set(library::FREETYPE, false);
		}

		if(to_deinitialize.test(library::ENET)) {
#if BUILD_ENET
			ensure(initialized.test(library::ENET));
			enet_deinitialize();
			initialized.set(library::ENET, false);
#endif
		}
	}

	global_libraries::~global_libraries() {
		deinit();
	}
};