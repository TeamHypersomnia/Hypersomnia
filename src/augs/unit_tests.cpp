#if BUILD_UNIT_TESTS
#define CATCH_CONFIG_RUNNER
#define CATCH_CONFIG_NOSTDOUT
#include <catch.hpp>
#endif

#include <sstream>

#include "augs/unit_tests.h"
#include "augs/misc/scope_guard.h"

namespace Catch {
	std::ostream& cout() {
		thread_local std::ostringstream s;
		return s;
	}

	std::ostream& cerr() {
		thread_local std::ostringstream s;
		return s;
	}

	std::ostream& clog() {
		thread_local std::ostringstream s;
		return s;
	}
}

namespace augs {
	void run_unit_tests(const unit_tests_settings& settings) {
#if BUILD_UNIT_TESTS
		if (!settings.run) {
			return;
		}

		auto clear_logs = make_scope_guard([]() {
			Catch::cout().clear();
			Catch::cerr().clear();
			Catch::clog().clear();
		});

		Catch::Session session;

		{
			auto& config = session.configData();

			config.showSuccessfulTests = settings.log_successful;
#if IS_PRODUCTION_BUILD
			config.shouldDebugBreak = false;
#else
			config.shouldDebugBreak = settings.break_on_failure;
#endif
			config.outputFilename = settings.redirect_log_to_path.string();
			config.runOrder = Catch::RunTests::InWhatOrder::InDeclarationOrder;
		}

		if (const auto result = session.run();
			result != 0
		) {
			auto e = unit_test_session_error(
				"Catch session failed with result: %x.",
				result
			);

			e.cout_content = dynamic_cast<std::ostringstream&>(Catch::cout()).str();
			e.cerr_content = dynamic_cast<std::ostringstream&>(Catch::cerr()).str();
			e.clog_content = dynamic_cast<std::ostringstream&>(Catch::clog()).str();

			throw e;
		}
#endif
	}
}