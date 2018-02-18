#if BUILD_UNIT_TESTS
#include <catch.hpp>
#include "augs/misc/pool/pool.h"
#include "augs/misc/constant_size_vector.h"

#include "augs/readwrite/memory_stream.h"
#include "augs/readwrite/byte_file.h"
#include "augs/math/vec2.h"
#include "augs/math/transform.h"
#include "augs/math/camera_cone.h"

static const auto path = GENERATED_FILES_DIR "test_byte_readwrite.bin";

template <class T>
static void report(const T& v, const T& reloaded) {
	if constexpr(can_stream_left_v<std::ostringstream, T>) {
		LOG("Original %x\nReloaded: %x", v, reloaded);
	}
}

template <class T>
bool report_compare(const T& tmp, const T& v) {
	const auto success = tmp == v;

	if (!success) {
		report(tmp, v);
	}

	return success;
}

template <class T>
static bool try_to_reload_with_file(T& v) {
	const T tmp = v;
	augs::save_as_bytes(v, path);
	augs::load_from_bytes(v, path);

	augs::remove_file(path);

	if constexpr(augs::is_pool_v<T>) {
		return true;
	}
	else {
		return report_compare(tmp, v);
	}
}

template <class T>
static bool try_to_reload_with_bytes(T& v) {
	const T tmp = v;
	{
		std::vector<std::byte> bytes;
		bytes = augs::to_bytes(v);

		augs::save_as_bytes(bytes, path);
	}

	augs::load_from_bytes(v, path);
	augs::remove_file(path);
	
	if constexpr(augs::is_pool_v<T>) {
		return true;
	}
	else {
		return report_compare(tmp, v);
	}
}

template <class T>
static bool try_to_reload_with_memory_stream(T& v) {
	const T tmp = v;
	{
		std::vector<std::byte> bytes;
		bytes = augs::to_bytes(v);

		augs::save_as_bytes(bytes, path);
	}

	auto bytes = augs::file_to_bytes(path);
	augs::memory_stream ss = std::move(bytes);
	augs::read_bytes(ss, v);

	augs::remove_file(path);

	if constexpr(augs::is_pool_v<T>) {
		return true;
	}
	else {
		return report_compare(tmp, v);
	}
}

#define test_cycle(variable) \
REQUIRE(try_to_reload_with_file(variable)); \
REQUIRE(try_to_reload_with_bytes(variable)); \
REQUIRE(try_to_reload_with_memory_stream(variable));

TEST_CASE("Byte readwrite Sanity check") {
	using T = std::variant<double, std::string, std::unordered_map<int, double>, std::optional<std::string>>;

	T a, b;
	REQUIRE(a == b);

	a = 2.0;
	b = 2.0;

	REQUIRE(a == b);
	
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

TEST_CASE("Byte readwrite Trivial types") {
	int a = 2;
	double b = 512.0;
	float C = 432.f;
	
	test_cycle(a);
	test_cycle(b);
	test_cycle(C);
}

TEST_CASE("Byte readwrite Classes") {
	vec2 ab;
	transform tr;
	std::vector<transform> v;
	v.resize(3);

	test_cycle(ab);
	test_cycle(tr);
	test_cycle(v);
}

TEST_CASE("Byte readwrite Arrays") {
	/* 
		Due to default initialization,
		arrays might have NANs, which would always return false upon comparison.
	*/

	std::array<float, 48> some{};
	std::array<std::array<float, 12>, 12> some_more{};
	std::array<std::array<std::array<std::array<std::array<float, 2>, 2>, 2>, 2>, 2> even_more{};

	test_cycle(some);
	test_cycle(some_more);
	test_cycle(even_more);
}

TEST_CASE("Byte readwrite Containers") {
	std::vector<float> abc;
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

	abc.resize(2);
	abcd.resize(2);
	abcde.resize(2);
	abcdef.resize(2);
	abcdef[0][412] = { 2, 3, 4 };
	abcdef[1][420] = { 997, 1 };

	test_cycle(mm);
	test_cycle(abc);
	test_cycle(abcd);
	test_cycle(abcde);
	test_cycle(abcdef);
}

TEST_CASE("Byte readwrite FixedContainers") {
	augs::constant_size_vector<int, 20> cc;
	cc.resize(8);
	test_cycle(cc);
	cc[0] = 483297;
	cc[1] = 478;
	cc[7] = 764;
	test_cycle(cc);
	cc.resize(2);
	test_cycle(cc);
	cc.resize(20);
	test_cycle(cc);
	cc[19] = 489;
	test_cycle(cc);
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

	test_cycle(abc);
	test_cycle(abcd);
	test_cycle(abcde);
	test_cycle(abcdef);
}

template <class T>
void test_pool() {
	T p;

	test_cycle(p);
	p.allocate(1);
	test_cycle(p);
	p.allocate(2);
	test_cycle(p);
	p.allocate(33);
	test_cycle(p);

	auto id = p.allocate(1);
	test_cycle(p);
	auto id2 = p.allocate(3);
	test_cycle(p);
	auto id3 = p.allocate(3);
	test_cycle(p);
	auto id4 = p.allocate(3);
	test_cycle(p);
	p.free(id2);
	test_cycle(p);
	p.free(id4);
	test_cycle(p);

	REQUIRE(p.find(id4) == nullptr);
	REQUIRE(p.find(id2) == nullptr);
	REQUIRE(p.find(id) != nullptr);
	REQUIRE(p.find(id3) != nullptr);

	REQUIRE(5 == p.size());
}

TEST_CASE("Byte readwrite Pools") {
	test_pool<augs::pool<float, of_size<100>::make_constant_vector, unsigned short>>();
	test_pool<augs::pool<float, std::vector, unsigned char>>();
}

TEST_CASE("Byte readwrite Variants and optionals") {
	using map_type = std::unordered_map<int, double>;
	using T = std::variant<double, std::string, map_type, std::optional<std::string>>;

	T v;

	{
		v = 2.0;
		test_cycle(v);
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
		test_cycle(v);
	}

	{
		std::string mm = "Hello world!";
		v = mm;
		test_cycle(v);
	}
}
#endif