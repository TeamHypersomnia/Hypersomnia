#if BUILD_UNIT_TESTS
#include <Catch/single_include/catch2/catch.hpp>
#include <tuple>
#include <unordered_map>

#include "augs/templates/introspection_utils/introspective_equal.h"
#include "augs/string/string_templates.h"

struct intro_test2 {
	// GEN INTROSPECTOR struct intro_test2
	std::unordered_map<int, float> it = { {1, 1.4f}, {2, 458.f} };
	bool blabla = true;
	char okay = 'A';
	// END GEN INTROSPECTOR
};

struct intro_test1 {
	// GEN INTROSPECTOR struct intro_test1
	int a = 4;
	int b = 3;
	std::array<int, 4> abc = { 1, 2, 4, 3 };
	std::tuple<int, double> tp = { 4, 5.0 }; 
	bool omg = false;
	intro_test2 it2;
	std::vector<int> av = { 458, 48, 31768 };
	// END GEN INTROSPECTOR
};

struct intro_test3 {
	// GEN INTROSPECTOR struct intro_test3
	int a = 1;
	int b = 2;
	// END GEN INTROSPECTOR
};

struct intro_test4 {
	// GEN INTROSPECTOR struct intro_test4
	int c = 3;
	int d = 4;
	// END GEN INTROSPECTOR
};

struct intro_test5 : intro_test3, intro_test4 {
	using introspect_bases = type_list<intro_test3, intro_test4>;
};

TEST_CASE("IntrospectionTest EqualityTest") {
	{
		intro_test2 t1;
		intro_test2 t2;

		REQUIRE(augs::introspective_equal(t1, t2));
		t1.it[4] = 6;
		REQUIRE(!augs::introspective_equal(t1, t2));
	}

	{
		intro_test1 t1;
		intro_test1 t2;

		REQUIRE(augs::introspective_equal(t1, t2));

		std::get<int>(t2.tp) = 5891;
		REQUIRE(!augs::introspective_equal(t1, t2));
	}

	{
		intro_test5 t1;
		intro_test5 t2;

		REQUIRE(augs::introspective_equal(t1, t2));

		t1.a = 5;
		REQUIRE(!augs::introspective_equal(t1, t2));

		t2.a = 5;
		REQUIRE(augs::introspective_equal(t1, t2));
	}
}

#endif
