#include "string_templates.h"

std::string to_forward_slashes(std::string str) {
	for (auto& s : str) {
		if (s == '\\') {
			s = '/';
		}
	}

	return str;
}

std::wstring to_forward_slashes(std::wstring str) {
	for (auto& s : str) {
		if (s == '\\') {
			s = '/';
		}
	}

	return str;
}

#if BUILD_UNIT_TESTS
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
