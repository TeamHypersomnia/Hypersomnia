#include "augs/build_settings/setting_build_unit_tests.h"

#if BUILD_UNIT_TESTS
#include <gtest/gtest.h>

#include "augs/math/vec2.h"
#include "augs/misc/typesafe_sscanf.h"

TEST(TypesafeSscanf, TypesafeSscanfSeveralTests) {
	{
		const auto format = "%x";
		const auto sprintfed = "1442";

		unsigned s1 = 0xdeadbeef;
		typesafe_sscanf(sprintfed, format, s1);

		EXPECT_EQ(1442, s1);
	}

	{
		const auto format = "%x,%x";
		const auto sprintfed = "1442,1337";

		unsigned s1 = 0xdeadbeef;
		unsigned s2 = 0xdeadbeef;
		typesafe_sscanf(sprintfed, format, s1, s2);

		EXPECT_EQ(1442, s1);
		EXPECT_EQ(1337, s2);
	}

	{
		const auto format = "%x,%x,%x:%x";
		const auto sprintfed = typesafe_sprintf(format, 1, 2, 3, 4);
		EXPECT_EQ("1,2,3:4", sprintfed);

		int s1, s2, s3, s4;
		typesafe_sscanf(sprintfed, format, s1, s2, s3, s4);

		EXPECT_EQ(1, s1);
		EXPECT_EQ(2, s2);
		EXPECT_EQ(3, s3);
		EXPECT_EQ(4, s4);
	}

	{


		vec2i test(123, -412);
		const auto format = "Vector is equal to: %x";
		const auto sprintfed = typesafe_sprintf(format, test);
		EXPECT_EQ("Vector is equal to: (123;-412)", sprintfed);

		vec2i read_test;

		typesafe_sscanf(sprintfed, format, read_test);

		EXPECT_EQ(test, read_test);
	}
}

#endif