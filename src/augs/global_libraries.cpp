#include <signal.h>
#if BUILD_ENET
#include <enet/enet.h>
#endif
#include <ft2build.h>
#include FT_FREETYPE_H

#include "augs/graphics/OpenGL_includes.h"

#include "global_libraries.h"
#include "augs/log.h"
#include "augs/ensure.h"
#include "augs/window_framework/platform_utils.h"

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

	void global_libraries::init(const library_flagset to_initialize) {
		// signal(SIGSEGV, sigsegv_handler);

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
			cfg.runOrder = Catch::RunTests::InWhatOrder::InDeclarationOrder;
		}

		const auto result = session.run(argc, argv);
		const bool was_catch_session_successful = result == 0;
		
		ensure(was_catch_session_successful);
#endif
	}
};

augs::global_libraries::library_flagset augs::global_libraries::initialized;
