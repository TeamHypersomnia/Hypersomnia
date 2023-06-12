
#if BUILD_UNIT_TESTS
#include <Catch/single_include/catch2/catch.hpp>

#include "augs/math/vec2.h"
#include "augs/string/typesafe_sscanf.h"
#include "augs/string/typesafe_sprintf.h"

template <typename... A>
int test_scanf(
	const std::string& source_string,
	const std::string& format,
	A&... a
) {
	return typesafe_scanf_detail(
		0, 
		0, 
		source_string,
		format,
		a...
	);
}

bool is_more_recent(const std::string& next_version, const std::string& current_version);

TEST_CASE("TypesafeSscanf", "TypesafeSscanfSeveralTests") {
	REQUIRE(!is_more_recent("", ""));
	REQUIRE(!is_more_recent("0.", ""));
	REQUIRE(!is_more_recent("", "0."));
	REQUIRE(!is_more_recent("klfjdlskj", "0klfjdlskj"));
	REQUIRE(!is_more_recent("1.0.0", "1.0.1"));
	REQUIRE(!is_more_recent("1.0.438", "1.1.0"));
	REQUIRE(is_more_recent("1.1.0", "1.0.438"));
	REQUIRE(is_more_recent("1.2.0", "1.1.8594"));
	REQUIRE(is_more_recent("1.2.0000", "1.1.8594"));
	REQUIRE(!is_more_recent("1.0.38", "1.0.38"));
	REQUIRE(!is_more_recent("2.0.0", "3.0.0"));
	REQUIRE(!is_more_recent("2.999.43", "3.0.0"));
	REQUIRE(is_more_recent("3.0.0", "2.999.43"));

	{
		const auto format = "%x";
		const auto sprintfed = "1442";

		unsigned s1 = 0xdeadbeef;
		REQUIRE(1 == test_scanf(sprintfed, format, s1));
		REQUIRE(1442 == s1);
	}

	{
		const auto format = "%x.%x.%x";
		const auto sprintfed = "1.0.1023";

		unsigned major = 0xdeadbeef;
		unsigned minor = 0xdeadbeef;
		unsigned revision = 0xdeadbeef;

		REQUIRE(3 == test_scanf(sprintfed, format, major, minor, revision));
		REQUIRE(major == 1);
		REQUIRE(minor == 0);
		REQUIRE(revision == 1023);
	}

	{
		const auto format = "%x,%x";
		const auto sprintfed = "1442,1337";

		unsigned s1 = 0xdeadbeef;
		unsigned s2 = 0xdeadbeef;
		REQUIRE(2 == test_scanf(sprintfed, format, s1, s2));

		REQUIRE(1442 == s1);
		REQUIRE(1337 == s2);
	}

	{
		const auto format = "%x,%x,%x:%x";
		const auto sprintfed = typesafe_sprintf(format, 1, 2, 3, 4);
		REQUIRE("1,2,3:4" == sprintfed);

		int s1, s2, s3, s4;
		REQUIRE(4 == test_scanf(sprintfed, format, s1, s2, s3, s4));

		REQUIRE(1 == s1);
		REQUIRE(2 == s2);
		REQUIRE(3 == s3);
		REQUIRE(4 == s4);
	}

	{
		int s1 = -1, s2 = -1;

		REQUIRE(1 == test_scanf(" 42", " %x %x", s1, s2));

		REQUIRE(s1 == 42);
		REQUIRE(s2 == -1);

		s1 = 22;
		REQUIRE(0 == test_scanf("", " %x %x", s1, s2));

		REQUIRE(s1 == 22);
		REQUIRE(s2 == -1);

		REQUIRE(0 == test_scanf("32 543", "", s1, s2));

		REQUIRE(s1 == 22);
		REQUIRE(s2 == -1);

		REQUIRE(1 == test_scanf("abc 543", "%x %x", s1, s2));

		REQUIRE(s1 == 22);
		REQUIRE(s2 == 543);

		REQUIRE(1 == test_scanf("4321 abc", "%x %x", s1, s2));

		REQUIRE(s1 == 4321);
		REQUIRE(s2 == 543);
	}

	{
		int a=-1, b=-1;
		std::string p;

		REQUIRE(3 == test_scanf("437% 233 - abcd", "%x% %x - %x", a, b, p));
		REQUIRE(a == 437);
		REQUIRE(b == 233);
		REQUIRE(p == "abcd");
	}
}

#endif