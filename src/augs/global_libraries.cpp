#include <signal.h>
#include <enet/enet.h>

#include <ft2build.h>
#include FT_FREETYPE_H

#include "augs/graphics/OpenGL_includes.h"

#include "global_libraries.h"
#include "augs/log.h"
#include "augs/ensure.h"
#include "augs/window_framework/platform_utils.h"
#include "generated/setting_build_unit_tests.h"

#if BUILD_UNIT_TESTS
#define CATCH_CONFIG_RUNNER
#include <catch.hpp>
#endif

void sigsegv_handler(const int signal) {
	augs::window::disable_cursor_clipping();
	throw "Access violation!";
}

namespace augs {
	std::unique_ptr<FT_Library> global_libraries::freetype_library(new FT_Library);

	void global_libraries::init(const library_bitset to_initialize) {
		// signal(SIGSEGV, sigsegv_handler);

		if(to_initialize.test(library::FREETYPE)) {
			ensure(!FT_Init_FreeType(freetype_library.get()) && "freetype initialization");
			initialized.set(library::FREETYPE);
		}
		
		if(to_initialize.test(library::ENET)) {
			ensure(enet_initialize() == 0 && L"Failed to initialize enet");
			initialized.set(library::ENET);
		}
	}

	void global_libraries::deinit(const library_bitset to_deinitialize) {
		if(to_deinitialize.test(library::FREETYPE)) {
			ensure(initialized.test(library::FREETYPE));
			ensure(!FT_Done_FreeType(*freetype_library.get()) && "freetype deinitialization");
			initialized.set(library::FREETYPE, false);
		}

		if(to_deinitialize.test(library::ENET)) {
			ensure(initialized.test(library::ENET));
			enet_deinitialize();
			initialized.set(library::ENET, false);
		}
	}

	void global_libraries::run_unit_tests(
		const int argc, 
		const char* const * const argv,
		const bool show_successful,
		const bool break_on_failure
	) {
#if BUILD_UNIT_TESTS
		Catch::Session session;

		{
			auto& cfg = session.configData();

			cfg.showSuccessfulTests = show_successful;
			cfg.shouldDebugBreak = break_on_failure;
			cfg.outputFilename = "generated/logs/unit_tests.txt";
		}

		session.run(argc, argv);
#endif
	}
};

augs::global_libraries::library_bitset augs::global_libraries::initialized;