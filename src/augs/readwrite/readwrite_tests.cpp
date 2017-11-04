#if BUILD_UNIT_TESTS
#include <catch.hpp>

#include "augs/filesystem/file.h"
#include "augs/readwrite/memory_stream.h"
#include "augs/readwrite/byte_readwrite.h"
#include "augs/math/vec2.h"
#include "augs/math/transform.h"
#include "augs/math/camera_cone.h"

static const auto path = GENERATED_FILES_DIR "test_byte_readwrite.bin";

template <class T>
static void try_to_reload_with_file(const T& v) {
	augs::save_as_bytes(v, path);
	T test;
	augs::load_from_bytes(test, path);

	augs::remove_file(path);
	REQUIRE(test == v);
}

template <class T>
static void try_to_reload_with_bytes(const T& v) {
	{
		std::vector<std::byte> bytes;
		bytes = augs::to_bytes(v);

		augs::save_as_bytes(bytes, path);
	}

	T test;
	augs::load_from_bytes(test, path);

	augs::remove_file(path);
	REQUIRE(test == v);
}

template <class T>
static void try_to_reload_with_memory_stream(const T& v) {
	{
		std::vector<std::byte> bytes;
		bytes = augs::to_bytes(v);

		augs::save_as_bytes(bytes, path);
	}

	T test;
	auto bytes = augs::file_to_bytes(path);
	augs::memory_stream ss = std::move(bytes);
	augs::read_bytes(ss, test);

	augs::remove_file(path);
	REQUIRE(test == v);
}

template <class T>
static void try_to_reload_with_all(const T& v) {
	try_to_reload_with_file(v);
	try_to_reload_with_bytes(v);
	try_to_reload_with_memory_stream(v);
}

TEST_CASE("Byte readwrite Trivial types") {
	int a = 2;
	double b = 512.0;
	float C = 432.f;

	try_to_reload_with_all(a);
	try_to_reload_with_all(b);
	try_to_reload_with_all(C);
}

TEST_CASE("Byte readwrite Classes") {
	vec2 ab;
	transform tr;
	std::vector<transform> v;
	v.resize(3);

	try_to_reload_with_all(ab);
	try_to_reload_with_all(tr);
	try_to_reload_with_all(v);
}

TEST_CASE("Byte readwrite Arrays") {
	std::array<float, 48> some;
	std::array<std::array<float, 12>, 12> some_more;
	std::array<std::array<std::array<std::array<std::array<float, 2>, 2>, 2>, 2>, 2> even_more;

	try_to_reload_with_all(some);
	try_to_reload_with_all(some_more);
	try_to_reload_with_all(even_more);
}

TEST_CASE("Byte readwrite Containers") {
	std::vector<float> abc;
	std::vector<int> abcd;
	std::vector<std::vector<int>> abcde;
	std::vector<std::unordered_map<int, std::vector<int>>> abcdef;
	abc.resize(2);
	abcd.resize(2);
	abcde.resize(2);
	abcdef.resize(2);
	abcdef[0][412] = { 2, 3, 4 };
	abcdef[1][420] = { 997, 1 };

	try_to_reload_with_all(abc);
	try_to_reload_with_all(abcd);
	try_to_reload_with_all(abcde);
	try_to_reload_with_all(abcdef);
}

TEST_CASE("Byte readwrite Variants and optionals") {
	using T = std::variant<double, std::string, std::unordered_map<int, double>, std::optional<std::string>>;

	T v;

	{
		v = 2.0;
		try_to_reload_with_all(v);
	}

	{
		std::unordered_map<int, double> mm;
		mm[4] = 2.0;
		mm[4287] = 455.2;
		mm[16445] = 4.0;
		v = mm;
		try_to_reload_with_all(v);
	}


	{
		std::optional<std::string> mm = "Hello world!";
		v = mm;
		try_to_reload_with_all(v);
	}

	{
		std::string mm = "Hello world!";
		v = mm;
		try_to_reload_with_all(v);
	}
}
#endif