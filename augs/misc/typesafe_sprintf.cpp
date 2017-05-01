#include "augs/build_settings/setting_build_unit_tests.h"

#if BUILD_UNIT_TESTS

#include <catch.hpp>

#include "augs/math/vec2.h"
#include "augs/misc/typesafe_sprintf.h"

TEST_CASE("Type-safe sprintf", "Several tests") {
	REQUIRE("1,2,3:4" == typesafe_sprintf("%x,%x,%x:%x", 1, 2, 3, 4));
	REQUIRE("abc,2,3:def" == typesafe_sprintf("%x,%x,%x:%x", "abc", 2, 3, "def"));
	REQUIRE("abc,2.55,3.14:def" == typesafe_sprintf("%x,%x,%x:%x", "abc", 2.55, 3.14f, "def"));

	vec2 test(123, 412);

	REQUIRE("Vector is equal to: (123;412)" == typesafe_sprintf("Vector is equal to: %x", test));

	int errid = 1282;
	std::string location = "augs::window::glwindow::create";

	REQUIRE("OpenGL error 1282 in augs::window::glwindow::create" == typesafe_sprintf("OpenGL error %x in %x", errid, location));
}
#endif
