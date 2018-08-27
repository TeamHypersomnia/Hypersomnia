#if BUILD_UNIT_TESTS
#include <vector>
#include <Catch/single_include/catch2/catch.hpp>
#include "augs/templates/container_templates.h"
#include "augs/templates/reversion_wrapper.h"
#include "augs/misc/constant_size_vector.h"

TEST_CASE("Templates EraseFromTo") {
	using v_t = std::vector<int>;
	v_t v = { 0, 1, 2, 3, 4 };
	erase_from_to(v, 0, 2);
	REQUIRE(v == v_t { 2, 3, 4 } );

	erase_from_to(v, 0);
	REQUIRE(v.empty());
	erase_from_to(v, 0);
	REQUIRE(v.empty());
}

TEST_CASE("Templates Reverse") {
	using v_t = std::vector<int>;
	v_t v = { 0, 1, 2, 3, 4 };

	std::string s;

	for (const auto& a : v) {
		s += std::to_string(a);
	}

	REQUIRE(s == "01234");

	for (const auto& a : reverse(v)) {
		s += std::to_string(a);
	}

	REQUIRE(s == "0123443210");

	{
		augs::constant_size_vector<int, 10> abc;
		abc.emplace_back(1);
		abc.emplace(abc.end(), 2);
		abc.insert(abc.end(), 4);
		abc.insert(abc.begin(), 8);

		REQUIRE(abc[0] == 8);
		REQUIRE(abc[1] == 1);
		REQUIRE(abc[2] == 2);
		REQUIRE(abc[3] == 4);
	}

	augs::constant_size_vector<int, 10> abc;

	abc.emplace_back(1);
	abc.emplace_back(2);
	abc.emplace_back(3);
	abc.emplace_back(4);
	abc.emplace_back(5);

	REQUIRE(abc.size() == 5);

	ping_pong_range(abc);

	REQUIRE(abc.size() == 10);

	REQUIRE(abc[0] == 1);
	REQUIRE(abc[1] == 2);
	REQUIRE(abc[2] == 3);
	REQUIRE(abc[3] == 4);
	REQUIRE(abc[4] == 5);

	REQUIRE(abc[5] == 5);
	REQUIRE(abc[6] == 4);
	REQUIRE(abc[7] == 3);
	REQUIRE(abc[8] == 2);
	REQUIRE(abc[9] == 1);
}
#endif
