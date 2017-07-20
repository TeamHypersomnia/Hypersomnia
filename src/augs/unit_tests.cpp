#include "unit_tests.h"

#if BUILD_UNIT_TESTS
#define CATCH_CONFIG_RUNNER
#include <catch.hpp>
#endif

#include "augs/ensure.h"
#include "application/debug_settings.h"

namespace augs {
	void run_unit_tests(
		const int argc,
		const char* const * const argv,
		const unit_tests_settings& settings
	) {
		if (!settings.run) {
			return;
		}

#if BUILD_UNIT_TESTS
		Catch::Session session;

		{
			auto& cfg = session.configData();

			cfg.showSuccessfulTests = settings.log_successful;
			cfg.shouldDebugBreak = settings.break_on_failure;
			cfg.outputFilename = settings.output_log_path;
			cfg.runOrder = Catch::RunTests::InWhatOrder::InDeclarationOrder;
		}

		const auto result = session.run(argc, argv);
		const bool was_catch_session_successful = result == 0;

		ensure(was_catch_session_successful);
#endif
	}
}