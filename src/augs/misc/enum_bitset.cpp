#include "generated/setting_build_unit_tests.h"

#if BUILD_UNIT_TESTS
#include "augs/misc/enum_bitset.h"
#include <catch.hpp>

TEST_CASE("CustomContainers EnumBitsetTest") {
	enum class aa {
		A,
		B,

		COUNT
	};

	{
		augs::enum_bitset<aa> myar{ aa::B };

		REQUIRE(myar.test(aa::B));
		REQUIRE(!myar.test(aa::A));
	}

	{
		augs::enum_bitset<aa> myar2{ aa::A, aa::B };

		REQUIRE(myar2.test(aa::A));
		REQUIRE(myar2.test(aa::B));
	}

	{
		augs::enum_bitset<aa> myar3{ aa::B, aa::A };

		REQUIRE(myar3.test(aa::B));
		REQUIRE(myar3.test(aa::A));
	}
}
#endif