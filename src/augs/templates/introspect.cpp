#if BUILD_UNIT_TESTS
#include <catch.hpp>
#include <tuple>
#include <unordered_map>

#include "augs/templates/introspect.h"
#include "augs/templates/string_templates.h"
#include "augs/misc/relinked_pool_id.h"

struct intro_test2 {
	// GEN INTROSPECTOR struct intro_test2
	std::unordered_map<int, float> it = { {1, 1.4f}, {2, 458.f} };
	bool blabla = true;
	char okay = 'A';
	relinked_pool_id<char> nice;
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

TEST_CASE("IntrospectionTest EqualityTest") {
	{
		intro_test2 t1;
		intro_test2 t2;

		REQUIRE(augs::recursive_equal(t1, t2));
		t1.it[4] = 6;
		REQUIRE(!augs::recursive_equal(t1, t2));
	}

	{
		intro_test1 t1;
		intro_test1 t2;

		REQUIRE(augs::recursive_equal(t1, t2));

		std::get<int>(t2.tp) = 5891;
		REQUIRE(!augs::recursive_equal(t1, t2));
	}
}

#endif
