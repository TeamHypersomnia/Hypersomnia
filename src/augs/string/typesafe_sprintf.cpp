#if BUILD_UNIT_TESTS
#include <Catch/single_include/catch2/catch.hpp>

#include "augs/math/vec2.h"
#include "augs/string/typesafe_sprintf.h"
#include "augs/string/string_templates.h"
#include "augs/filesystem/path.h"

TEST_CASE("Type-safe sprintf", "Several tests") {
	REQUIRE("foo/bar/nice.vsh" == to_forward_slashes(typesafe_sprintf("%x/%x", augs::path_type("foo/bar"), "nice.vsh")));

	// corner cases
	REQUIRE("%x%x%%%%f%c%ddasdfs" == typesafe_sprintf("%x%x%%%%f%c%ddasdfs"));
	REQUIRE("2,3,%x" == typesafe_sprintf("%x,%x,%x", 2, 3));
	REQUIRE("2,3,5" == typesafe_sprintf("%x,%x,%x", 2, 3, 5, 7, 8, 6, 5, 435, 534, 324534, "nice"));

	REQUIRE("1,2,3:4" == typesafe_sprintf("%x,%x,%x:%x", 1, 2, 3, 4));
	REQUIRE("abc,2,3:def" == typesafe_sprintf("%x,%x,%x:%x", "abc", 2, 3, "def"));
	REQUIRE("abc,2.55,3.14:def" == typesafe_sprintf("%x,%x,%x:%x", "abc", 2.55, 3.14f, "def"));

	vec2 test(123, 412);

	REQUIRE("Vector is equal to: (123;412)" == typesafe_sprintf("Vector is equal to: %x", test));
	REQUIRE("Vector is equal to: (123.00;412.00)" == typesafe_sprintf("Vector is equal to: (%f2;%f2)", test.x, test.y));

	REQUIRE("Syncing test (2/12): 40.00% complete. (Total: 30.00%)" == typesafe_sprintf("Syncing %x (%x/%x): %2f% complete. (Total: %2f%)", "test", 2, 12, 100 * 0.4, 100 * 0.3));

	int errid = 1282;
	std::string location = "augs::create";

	REQUIRE("OpenGL error 1282 in augs::create" == typesafe_sprintf("OpenGL error %x in %x", errid, location));

	REQUIRE("%" == typesafe_sprintf("%", 1,2,3));
	REQUIRE("%%" == typesafe_sprintf("%%", 1,2,3));
	REQUIRE("%%3" == typesafe_sprintf("%%%x", 3,2,1));
	REQUIRE("" == typesafe_sprintf(""));
	REQUIRE("%%%%%%%%%%%%b" == typesafe_sprintf("%%%%%%%%%%%%b"));
	REQUIRE("1.23" == typesafe_sprintf("%f2", 1.234567));
	REQUIRE("3.14" == typesafe_sprintf("%2f", 3.14159265));

	std::array<std::byte, 3> bb = { 
		static_cast<std::byte>(1), static_cast<std::byte>(34), static_cast<std::byte>(255)
	};

	REQUIRE("[1 34 255]" == typesafe_sprintf("[%x]", format_as_bytes(bb)));
}
#endif
