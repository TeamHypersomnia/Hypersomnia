#include <cstdint>
#include "augs/string/typesafe_sscanf.h"

bool is_more_recent(const std::string& next_version, const std::string& current_version) {
	int major_a = 0;
	int minor_a = 0;
	int revision_a = 0;
	int major_b = 0;
	int minor_b = 0;
	int revision_b = 0;

	const auto format = "%x.%x.%x";

	if (
		typesafe_sscanf(   next_version, format, major_a, minor_a, revision_a)
		&& typesafe_sscanf(current_version, format, major_b, minor_b, revision_b)

	) {
		if (major_a != major_b) {
			return major_a > major_b;
		}
		if (minor_a != minor_b) {
			return minor_a > minor_b;
		}
		if (revision_a != revision_b) {
			return revision_a > revision_b;
		}
	}

	return false;
}

#if BUILD_UNIT_TESTS
#include <Catch/single_include/catch2/catch.hpp>

#include "augs/math/vec2.h"
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

TEST_CASE("IsMoreRecent", "IsMoreRecentSeveralTests") {
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
}

TEST_CASE("TypesafeSscanf", "TypesafeSscanfSeveralTests") {

	{
		const auto format = "";
		const auto sprintfed = "";

		unsigned s1 = 0xdeadbeef;
		REQUIRE(0 == test_scanf(sprintfed, format, s1));
		REQUIRE(s1 == 0xdeadbeef);
	}

	{
		const auto format = "1234";
		const auto sprintfed = "";

		unsigned s1 = 0xdeadbeef;
		REQUIRE(0 == test_scanf(sprintfed, format, s1));
		REQUIRE(s1 == 0xdeadbeef);
	}

	{
		const auto format = "%x";
		const auto sprintfed = "1";

		unsigned s1 = 0xdeadbeef;
		REQUIRE(1 == test_scanf(sprintfed, format, s1));
		REQUIRE(s1 == 1);
	}

	{
		const auto format = "%x";
		const auto sprintfed = "-1";

		unsigned s1 = 0xdeadbeef;
		REQUIRE(1 == test_scanf(sprintfed, format, s1));
		REQUIRE(s1 == uint32_t(-1));
	}

	{
		const auto format = "";
		const auto sprintfed = "1234";

		unsigned s1 = 0xdeadbeef;
		REQUIRE(0 == test_scanf(sprintfed, format, s1));
		REQUIRE(s1 == 0xdeadbeef);
	}

	{
		const auto format = "";
		const auto sprintfed = "%x";

		unsigned s1 = 0xdeadbeef;
		REQUIRE(0 == test_scanf(sprintfed, format, s1));
		REQUIRE(s1 == 0xdeadbeef);
	}

	{
		const auto format = "%x";
		const auto sprintfed = "%x";

		unsigned s1 = 0xdeadbeef;
		REQUIRE(0 == test_scanf(sprintfed, format, s1));
		REQUIRE(s1 == 0xdeadbeef);
	}

	{
		const auto format = "%x";
		const auto sprintfed = "%x";

		std::string s1;
		REQUIRE(1 == test_scanf(sprintfed, format, s1));
		REQUIRE(s1 == "%x");
	}

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

	{
		int a=-1, b=-1;

		REQUIRE(1 == test_scanf("/tutorial/4", "/tutorial/%x", a));
		REQUIRE(a == 4);
		REQUIRE(0 == test_scanf("/tutorial/", "/tutorial/%x", a));
		REQUIRE(0 == test_scanf("/tutorial", "/tutorial/%x", a));
		REQUIRE(0 == test_scanf("/tutorial4343", "/tutorial/%x", a));
		REQUIRE(1 == test_scanf("/tutorial/8888/43543", "/tutorial/%x", a));
		REQUIRE(a == 8888);
		REQUIRE(2 == test_scanf("/tutorial/1/2", "/tutorial/%x/%x", a, b));
		REQUIRE(a == 1);
		REQUIRE(b == 2);
		REQUIRE(2 == test_scanf("/tutorial/3/4/", "/tutorial/%x/%x", a, b));
		REQUIRE(a == 3);
		REQUIRE(b == 4);
	}

	{
		int id = -1;
		std::string rest;
		REQUIRE(2 == test_scanf("[CH] arena-ch.hypersomnia.io (non-ranked) #3", "%x#%x", rest, id));
		REQUIRE(3 == id);
		REQUIRE(2 == test_scanf("[US] arena-us.hypersomnia.io #1", "%x#%x", rest, id));
		REQUIRE(1 == id);

		REQUIRE(2 == test_scanf("[CH] Switzerland #8 R", "%x#%x", rest, id));
		REQUIRE(8 == id);
	}
}

#endif