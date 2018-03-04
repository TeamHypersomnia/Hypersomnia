#if BUILD_UNIT_TESTS
#include <vector>
#include <catch.hpp>
#include "augs/templates/container_templates.h"

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
}
#endif
