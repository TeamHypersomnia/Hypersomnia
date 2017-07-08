
#if BUILD_UNIT_TESTS
#include "string_templates.h"
#include <catch.hpp>

TEST_CASE("Templates StringTemplates") {
	{
		std::string test = "abababthis<abc> 8947208dj;;;abab</cdcde> isabab<cd>abcd</cd> the expected result";
		str_ops(test)
			.multi_replace_all({ "ab", "cd" }, "")
			.replace_all("<c> 8947208dj;;;</e>", "")
			.replace_all("<></>", "")
			;

		REQUIRE(test == "this is the expected result");
	}

	{
		std::string test = "abc";
		str_ops(test)
			.multi_replace_all({ "abc" }, "")
		;

		REQUIRE(test.empty());
	}

	{
		std::string test = "";
		str_ops(test)
			.multi_replace_all({ " " }, "")
		;

		REQUIRE(test.empty());
	}
}
#endif