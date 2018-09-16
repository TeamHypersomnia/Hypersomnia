#if BUILD_UNIT_TESTS
#include "augs/misc/enum/enum_map.h"
#include <Catch/single_include/catch2/catch.hpp>

TEST_CASE("EnumMap") {
	enum class tenum {
		_0,
		_1,
		_2,
		_3,
		_4,
		COUNT
	};

	augs::enum_map<tenum, int> mm;
	mm[tenum::_0] = 0;
	mm[tenum::_1] = 1;
	mm[tenum::_2] = 2;
	
	int cnt = 0;

	for (const auto&& m : mm) {
		REQUIRE(m.first == tenum(m.second));
		REQUIRE(m.second == cnt++);
	}

	REQUIRE(3 == cnt);

	for (const auto&& m : reverse(mm)) {
		REQUIRE(m.first == tenum(m.second));
		REQUIRE(m.second == --cnt);
	}

	REQUIRE(0 == cnt);

	{
		augs::enum_map<tenum, int> emp;

		for (const auto&& abc : emp) {
			REQUIRE(false);
			(void)abc;
		}

		for (const auto&& abc : reverse(emp)) {
			REQUIRE(false);
			(void)abc;
		}

		REQUIRE(emp.size() == 0);
		emp[tenum::_0] = 48;
		REQUIRE(emp.size() == 1);

		for (const auto&& m : reverse(emp)) {
			REQUIRE(48 == m.second);
		}

		emp.clear();
		REQUIRE(emp.size() == 0);
		emp[tenum::_1] = 84;
		REQUIRE(emp.size() == 1);

		for (const auto&& m : reverse(emp)) {
			REQUIRE(84 == m.second);
		}

		emp[tenum::_0] = 0;
		emp[tenum::_1] = 1;
		emp[tenum::_2] = 2;

		REQUIRE(emp.size() == 3);

		for (const auto&& m : emp) {
			REQUIRE(m.first == tenum(m.second));
			REQUIRE(m.second == cnt++);
		}

		REQUIRE(3 == cnt);

		for (const auto&& m : reverse(emp)) {
			REQUIRE(m.first == tenum(m.second));
			REQUIRE(m.second == --cnt);
		}

		emp.clear();
		REQUIRE(emp.size() == 0);
		emp[tenum::_4] = 4;
		emp[tenum::_3] = 3;
		emp[tenum::_1] = 1;
		emp[tenum::_0] = 0;

		auto it = emp.rbegin();
		REQUIRE((*it).second == 4);
		REQUIRE((*++it).second == 3);
		REQUIRE((*++it).second == 1);
		REQUIRE((*++it).second == 0);
		REQUIRE(++it == emp.rend());
	}
}

#endif
