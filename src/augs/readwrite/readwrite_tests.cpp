#if BUILD_UNIT_TESTS
#include <catch.hpp>

#include "augs/filesystem/file.h"
#include "augs/readwrite/memory_stream.h"
#include "augs/readwrite/byte_readwrite.h"

TEST_CASE("Byte readwrite", "Several tests") {
	using T = std::variant<double, std::string, std::unordered_map<int, double>, std::optional<std::string>>;
	const auto path = GENERATED_FILES_DIR "test_byte_readwrite.bin";

	T v;

	auto test_cycle = [&]() {
		{
			augs::save_as_bytes(v, path);
			T test;
			augs::load_from_bytes(test, path);

			REQUIRE(test == v);
		}

		{
			{
				std::vector<std::byte> bytes;
				bytes = augs::to_bytes(v);
				
				augs::save_as_bytes(bytes, path);
			}

			T test;
			augs::load_from_bytes(test, path);

			REQUIRE(test == v);
		}

		{
			{
				std::vector<std::byte> bytes;
				bytes = augs::to_bytes(v);

				augs::save_as_bytes(bytes, path);
			}

			T test;
			auto bytes = augs::file_to_bytes(path);
			augs::memory_stream ss = std::move(bytes);
			augs::read_bytes(ss, test);

			REQUIRE(test == v);
		}
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