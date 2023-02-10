#if !IS_PRODUCTION_BUILD
#if BUILD_UNIT_TESTS
#include "augs/log.h"
#include <Catch/single_include/catch2/catch.hpp>
#include "augs/misc/pool/pool_io.hpp"
#include "augs/misc/pool/pool.h"
#include "augs/misc/pool/pool_allocate.h"
#include "augs/misc/constant_size_vector.h"
#include "augs/misc/enum/enum_array.h"

#include "augs/string/string_templates.h"
#include "augs/readwrite/readwrite_test_cycle.h"

#include "augs/math/vec2.h"
#include "augs/math/transform.h"
#include "augs/math/camera_cone.h"
#include "augs/misc/enum/enum_boolset.h"
#include "augs/misc/constant_size_string.h"

TEST_CASE("Filesystem test") {
	const auto& path = test_file_path;

	augs::save_as_text(path, "");
	REQUIRE(augs::exists(path));
	REQUIRE(!augs::file_exists_and_non_empty(path));

	augs::save_as_text(path, "a");
	REQUIRE(augs::exists(path));
	REQUIRE(augs::file_exists_and_non_empty(path));

	augs::remove_file(path);
	REQUIRE(!augs::exists(path));
	REQUIRE(!augs::file_exists_and_non_empty(path));
}

TEST_CASE("Byte readwrite Sanity check") {
	using T = std::variant<std::monostate, double, std::string, std::unordered_map<int, double>, std::optional<std::string>>;

	{
		detail::dummy_C a, b;
		REQUIRE(a == b);
		REQUIRE(detail::dummy_A() == detail::dummy_A());
		REQUIRE(detail::dummy_C() == detail::dummy_C());

		detail::dummy_B c, d;
		REQUIRE(c == d);
	}

	T a, b;
	REQUIRE(a == b);

	a = 2.0;
	b = 2.0;

	REQUIRE(a == b);
	REQUIRE(static_cast<float>(0) == 0.f);
	
	REQUIRE(std::optional<std::string>() == std::optional<std::string>());

	a = std::optional<std::string>();
	b = std::optional<std::string>();

	REQUIRE(a == b);
	
	REQUIRE((std::unordered_map<int, double>() == std::unordered_map<int, double>()));

	std::unordered_map<int, double> ma;
	ma[2] = 4587.0;
	ma[3] = 458247.0;
	ma[22] = 45187.0;
	std::unordered_map<int, double> mb;
	mb[2] = 4587.0;
	mb[3] = 458247.0;
	mb[22] = 45187.0;

	REQUIRE(ma == mb);

	using arr = std::array<std::array<std::array<std::array<std::array<float, 2>, 2>, 2>, 2>, 2>;
	REQUIRE(arr() == arr());
}

TEST_CASE("Byte readwrite Custom structs") {
	detail::dummy_A a;
	detail::dummy_B b;
	detail::dummy_C c;

	readwrite_test_cycle(a);
	readwrite_test_cycle(b);
	readwrite_test_cycle(c);

	a.a = 34;
	b.a = 5890;
	c.a = 543;

	readwrite_test_cycle(a);
	readwrite_test_cycle(b);
	readwrite_test_cycle(c);

	a.a = 324;
	b.b = 590;
	c.c = 548;

	readwrite_test_cycle(a);
	readwrite_test_cycle(b);
	readwrite_test_cycle(c);
}
TEST_CASE("Byte readwrite Trivial types") {
	int a = 2;
	double b = 512.0;
	float C = 432.f;
	
	readwrite_test_cycle(a);
	readwrite_test_cycle(b);
	readwrite_test_cycle(C);
}

TEST_CASE("Byte readwrite Classes") {
	vec2 ab;
	transformr tr;
	std::vector<transformr> v;
	v.resize(3);

	readwrite_test_cycle(ab);
	readwrite_test_cycle(tr);
	readwrite_test_cycle(v);
}

TEST_CASE("Byte readwrite Arrays") {
	/* 
		Due to default initialization,
		arrays might have NANs, which would always return false upon comparison.
	*/

	std::array<float, 48> some{};
	std::array<std::array<float, 12>, 12> some_more{};
	std::array<std::array<std::array<std::array<std::array<float, 2>, 2>, 2>, 2>, 2> even_more{};

	enum ttt {
		A, B, C, COUNT
	};

	augs::enum_array<std::vector<int>, ttt> eee = {};
	eee[ttt::C] = { 43 };
	eee[ttt::A] = { 143 };

	std::array<std::vector<int>, 4> ahaha = {};
	ahaha[0] = {};
	ahaha[2] = { 24, 444 };

	readwrite_test_cycle(some);
	readwrite_test_cycle(some_more);
	readwrite_test_cycle(even_more);
	readwrite_test_cycle(eee);
	readwrite_test_cycle(ahaha);
}

TEST_CASE("Byte readwrite Containers") {
	std::vector<float> abc;
	std::vector<float> abcf;
	std::vector<int> abcd;
	std::vector<std::vector<int>> abcde;
	std::vector<std::unordered_map<int, std::vector<int>>> abcdef;
	std::unordered_map<int, double> mm;
	
	static_assert(is_container_v<decltype(mm)>);
	static_assert(is_container_v<decltype(abcdef)>);
	
	static_assert(is_associative_v<decltype(mm)>);

	mm[4] = 2.0;
	mm[4287] = 455.2;
	mm[16445] = 4.0;

	abc.resize(2, 9.f);
	abcf.resize(2);
	abcd.resize(2);
	abcde.resize(2);
	abcdef.resize(2);
	abcdef[0][412] = { 2, 3, 4 };
	abcdef[1][420] = { 997, 1 };

	readwrite_test_cycle(mm);
	readwrite_test_cycle(abc);
	readwrite_test_cycle(abcf);
	readwrite_test_cycle(abcd);
	readwrite_test_cycle(abcde);
	readwrite_test_cycle(abcdef);
}

TEST_CASE("Byte readwrite FixedContainers") {
	{
		augs::constant_size_string<5> abab = "hja";
		REQUIRE(std::string(abab) == "hja");
		readwrite_test_cycle(abab);
		REQUIRE(std::string(abab) == "hja");

		abab = "ohyeah";
		// Check trimming
		REQUIRE(std::string(abab) == "ohyea");
		readwrite_test_cycle(abab);
	}

	augs::constant_size_vector<int, 20> cc;

	cc.resize(8);
	readwrite_test_cycle(cc);
	cc[0] = 483297;
	cc[1] = 478;
	cc[7] = 764;
	readwrite_test_cycle(cc);
	cc.resize(2);
	readwrite_test_cycle(cc);
	cc.resize(20);
	readwrite_test_cycle(cc);
	cc[19] = 489;
	readwrite_test_cycle(cc);

	using E = detail::dummy_enum;
	augs::enum_boolset<E> bb;
	readwrite_test_cycle(bb);

	bb[E::_3] = true;
	bb[E::_4] = true;

	readwrite_test_cycle(bb);
}

TEST_CASE("Byte readwrite Optionals") {
	std::optional<std::vector<float>> abc = std::vector<float>();
	std::optional<std::vector<int>> abcd = std::vector<int>();
	std::optional<std::vector<std::vector<int>>> abcde = std::vector<std::vector<int>>();
	std::vector<std::unordered_map<int, std::optional<std::vector<int>>>> abcdef;
	abc->resize(2);
	abcd->resize(2);
	abcde->resize(2);
	abcdef.resize(2);
	abcdef[0][412] = { { 2, 3, 4 } };
	abcdef[1][420] = { { 997, 1 } };

	readwrite_test_cycle(abc);
	readwrite_test_cycle(abcd);
	readwrite_test_cycle(abcde);
	readwrite_test_cycle(abcdef);
}

TEST_CASE("Byte readwrite Pointers") {
	std::unique_ptr<std::vector<float>> abc(new std::vector<float>());
	std::unique_ptr<std::vector<int>> abcd(new std::vector<int>());
	std::unique_ptr<std::vector<std::vector<int>>> abcde(new std::vector<std::vector<int>>());

	abc->resize(2);
	abcd->resize(2);
	abcde->resize(2);

	readwrite_test_cycle(abc);
	readwrite_test_cycle(abcd);
	readwrite_test_cycle(abcde);
}

TEST_CASE("Lua readwrite General") {
	auto lua = augs::create_lua_state();
	
	using E = detail::dummy_enum;
	augs::enum_boolset<E> bb;
	readwrite_test_cycle(bb);

	bb[E::_3] = true;
	bb[E::_4] = true;

	augs::simple_pair<int, bool> dm = { 4325, true };
	std::vector<int> abc = { 23, 4, 523 };

	REQUIRE(try_to_reload_with_lua(lua, bb));
	REQUIRE(try_to_reload_with_lua(lua, dm));
	REQUIRE(try_to_reload_with_lua(lua, abc));
	REQUIRE(try_to_reload_with_lua(lua, bb));

	{
		using map_type = std::unordered_map<int, std::string>;
		using T = std::variant<std::monostate, int, std::string, map_type, std::pair<std::string, int>>;

		T with_monostate;
		REQUIRE(try_to_reload_with_lua(lua, with_monostate));

		map_type mm;
		mm[4] = "2.0";
		mm[4287] = "455.2";
		mm[16445] = "4.0";
		T v = mm;

		REQUIRE(try_to_reload_with_lua(lua, mm));
		REQUIRE(try_to_reload_with_lua(lua, v));

		using P = augs::pool<T, of_size<300>::make_nontrivial_constant_vector, unsigned short>;
		P pp;
		pp.reserve(300);

		pp.allocate(T());
		pp.allocate(T());
		pp.allocate(v);
		pp.free(pp.allocate(T()));

		const auto before = pp;
		REQUIRE(try_to_reload_with_lua(lua, pp));
		REQUIRE(try_to_reload_with_lua(lua, pp));
		REQUIRE(try_to_reload_with_lua(lua, pp));

		auto make_pool_bytes = [&](const auto& from) {
			std::vector<std::byte> b;
			auto ss = augs::ref_memory_stream(b);

			from.for_each_id_and_object(
				[&](const auto& a, const auto& b){
					augs::write_bytes(ss, a);
					augs::write_bytes(ss, b);
				}
			);

			return b;
		};

		{
			const auto ba = make_pool_bytes(before);
			const auto bb = make_pool_bytes(pp);

			REQUIRE(ba == bb);
		}
	}
}

TEST_CASE("Byte readwrite Variants and optionals") {
	const auto& path = test_file_path;

	using map_type = std::unordered_map<int, double>;
	using T = std::variant<std::monostate, double, std::string, map_type, std::optional<std::string>>;

	T v;

	{
		v = 2.0;
		readwrite_test_cycle(v);
		v = std::monostate();
		readwrite_test_cycle(v);
	}

	{
		map_type mm;
		mm[4] = 2.0;
		mm[4287] = 455.2;
		mm[16445] = 4.0;
		v = mm;

		T by_assignment;
		by_assignment = mm;

		REQUIRE(by_assignment == v);

		augs::save_as_bytes(v, path);
		T test;
		augs::load_from_bytes(test, path);

		augs::remove_file(path);

		REQUIRE(test.index() == v.index());

		{
			const auto& m1 = std::get<map_type>(test);
			const auto& m2 = std::get<map_type>(v);
		
			REQUIRE(m1.size() == m2.size());
			
			if (m1 != m2) {
				LOG("reloaded:");
				
				for (const auto& it : m1) {
					LOG("[%x] = %x (%x)", it.first, it.second, format_as_bytes(it.second));
				}

				LOG("original:");

				for (const auto& it : m2) {
					LOG("[%x] = %x (%x)", it.first, it.second, format_as_bytes(it.second));
				}
			}

			REQUIRE(m1.at(4) == m2.at(4));
			REQUIRE(m1.at(4287) == m2.at(4287));
			REQUIRE(m1.at(16445) == m2.at(16445));
		
			REQUIRE(m1 == m2);
		}

		REQUIRE(v == test);
	}


	{
		std::optional<std::string> mm = "Hello world!";
		v = mm;
		readwrite_test_cycle(v);
	}

	{
		std::string mm = "Hello world!";
		v = mm;
		readwrite_test_cycle(v);
	}
}
#endif
#endif