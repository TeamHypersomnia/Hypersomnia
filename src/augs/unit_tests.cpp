#if BUILD_UNIT_TESTS
#define CATCH_CONFIG_RUNNER
#include <catch.hpp>
#endif

#include "augs/ensure.h"
#include "augs/unit_tests.h"

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
			auto& config = session.configData();

			config.showSuccessfulTests = settings.log_successful;
			config.shouldDebugBreak = settings.break_on_failure;
			config.outputFilename = settings.output_log_path;
			config.runOrder = Catch::RunTests::InWhatOrder::InDeclarationOrder;
		}

		const auto result = session.run(argc, argv);
		const bool was_catch_session_successful = result == 0;

		ensure(was_catch_session_successful);
#endif
	}
}