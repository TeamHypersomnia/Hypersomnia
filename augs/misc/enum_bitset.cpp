#include "augs/build_settings/setting_build_gtest.h"

#if BUILD_GTEST
#include "augs/misc/enum_bitset.h"
#include <gtest/gtest.h>

TEST(CustomContainers, EnumBitsetTest) {
	enum class aa {
		A,
		B,

		COUNT
	};

	{
		augs::enum_bitset<aa> myar{ aa::B };

		EXPECT_TRUE(myar.test(aa::B));
		EXPECT_FALSE(myar.test(aa::A));
	}

	{
		augs::enum_bitset<aa> myar2{ aa::A, aa::B };

		EXPECT_TRUE(myar2.test(aa::A));
		EXPECT_TRUE(myar2.test(aa::B));
	}

	{
		augs::enum_bitset<aa> myar3{ aa::B, aa::A };

		EXPECT_TRUE(myar3.test(aa::B));
		EXPECT_TRUE(myar3.test(aa::A));
	}
}
#endif