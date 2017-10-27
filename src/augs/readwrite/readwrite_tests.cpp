#if BUILD_UNIT_TESTS
#include <catch.hpp>

#include "augs/filesystem/file.h"
#include "augs/readwrite/byte_readwrite.h"

TEST_CASE("Byte readwrite", "Several tests") {
	using T = std::variant<double, std::string, std::unordered_map<int, double>, std::optional<std::string>>;
	const auto path = GENERATED_FILES_DIR "test_byte_readwrite.bin";

	T v;

	auto test_cycle = [&]() {
		augs::save(v, path);
		T test;
		augs::load(test, path);

		REQUIRE(test == v);
	};

	{
		v = 2.0;
		test_cycle();
	}

	{
		std::unordered_map<int, double> mm;
		mm[4] = 2.0;
		mm[4287] = 455.2;
		mm[16445] = 4.0;
		v = mm;
		test_cycle();
	}


	{
		std::optional<std::string> mm = "Hello world!";
		v = mm;
		test_cycle();
	}

	{
		std::string mm = "Hello world!";
		v = mm;
		test_cycle();
	}

	augs::remove_file(path);
}
#endif