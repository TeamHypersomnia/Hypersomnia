#include <cstddef>
#include "augs/misc/compress.h"
#include "lz4.h"
#include "augs/log.h"
#include "augs/templates/container_templates.h"

#define DISABLE_COMPRESSION 0

#if DISABLE_COMPRESSION
#else
#define ENABLE_COMPRESSION 1
#endif

namespace augs {
	std::vector<std::byte> make_compression_state() {
		std::vector<std::byte> out;
		out.resize(LZ4_sizeofState());
		return out;
	}

	std::vector<std::byte> compress(
		std::vector<std::byte>& state,
		const std::vector<std::byte>& input
	) {
		std::vector<std::byte> output;
		compress(state, input, output);
		return output;
	}

	void compress(
		std::vector<std::byte>& state,
		const std::vector<std::byte>& input,
		std::vector<std::byte>& output
	) {
		compress(state, input.data(), input.size(), output);
	}

	void compress(
		std::vector<std::byte>& state,
		const std::byte* input,
		const std::size_t input_size,
		std::vector<std::byte>& output
	) {
#if DISABLE_COMPRESSION
		(void)state;
		output.insert(output.end(), input, input + input_size);
#else
		const auto size_bound = LZ4_compressBound(input_size);
		const auto prev_size = output.size();
		output.resize(prev_size + size_bound);

		const auto bytes_written = LZ4_compress_fast_extState(
			reinterpret_cast<void*>(state.data()), 
			reinterpret_cast<const char*>(input), 
			reinterpret_cast<char*>(output.data() + prev_size), 
			input_size,
			size_bound,
			1
		);

		output.resize(prev_size + bytes_written);
#endif
	}

	std::vector<std::byte> decompress(
		const std::vector<std::byte>& input,
		const std::size_t uncompressed_size
	) {
		std::vector<std::byte> output;
		output.resize(uncompressed_size);

		decompress(input, output);
		return output;
	}

	void decompress(
		const std::vector<std::byte>& input,
		std::vector<std::byte>& output
	) {
		decompress(input.data(), input.size(), output);
	}

	void decompress(
		const std::byte* const input,
		const std::size_t byte_count,
		std::vector<std::byte>& output
	) {
#if DISABLE_COMPRESSION
		output.assign(input, input + byte_count);
#else
		const auto uncompressed_size = output.size();

		const auto bytes_read = LZ4_decompress_safe(
			reinterpret_cast<const char*>(input), 
			reinterpret_cast<char*>(output.data()), 
			byte_count,
			static_cast<int>(uncompressed_size)
		);

		if (bytes_read < 0) {
			output.clear();
			throw decompression_error("CHECK IF YOU PASSED CORRECT uncompressed_size! Decompression failure. Failed to read any bytes.");
		}

		if (uncompressed_size != static_cast<std::size_t>(bytes_read)) {
			output.clear();
			throw decompression_error("CHECK IF YOU PASSED CORRECT uncompressed_size! Decompression failure. Read %x bytes, but expected %x.", bytes_read, uncompressed_size);
		}
#endif
	}
}

#if BUILD_UNIT_TESTS
#include <Catch/single_include/catch2/catch.hpp>

TEST_CASE("Ca CompressionDecompression") {
	const auto tests = {
		"",
		"jsdfhsdkj fhdslkjfh37824y239874y298734y9382ejhfdffgsdfgsdfgsfdgdsfglkjdfsjglskdfjglksdfjgsdfgjhksdfj ghsdjfghsdjkfghsjfkdghskdfgkfsdjghkj",
		"jsdfhsdkj 8923749087sdewlak fidsuj 89r2 jklsdfjj 89023jfoui23jfjhglkddjgh87934ylalksja;lsdalf89kl43jvgfh7823dsakjhd78t3kjhsd78",
		"	aslkd jaslkajslk87679608759086476980574069854lkglljgj0219238nmmlsdfjlkq290831ulmdskmfsn 8329iuflmdskzmaoq091!()@*!)(@*!($%*%!&(%*&(*8ghsdjfghsdjkfghsjfkdghskdfgkfsdjghkj",
		"1234",
		"89750983275098437508934759043",
		"00000000"
	};

	for (const auto& t : tests) {
		auto blahblah = std::string(t);

		std::vector<std::byte> input;
		input.assign(
			reinterpret_cast<const std::byte*>(blahblah.data()), 
			reinterpret_cast<const std::byte*>(blahblah.data() + blahblah.size())
		);

		auto state = augs::make_compression_state();
		const auto compressed = augs::compress(state, input);

#if DISABLE_COMPRESSION
		REQUIRE(compressed == input);
#else
		REQUIRE(compressed != input);
#endif

		{
			const auto decompressed = augs::decompress(augs::compress(state, input), input.size());

			REQUIRE(decompressed == input);
		}

		{
			std::vector<std::byte> bad_decompressed;
			bad_decompressed.resize(input.size() + 1);

			try {
				augs::decompress(augs::compress(state, input), bad_decompressed);
#if ENABLE_COMPRESSION
				REQUIRE(false);
#endif
			}
			catch(const augs::decompression_error&) { 
				REQUIRE(bad_decompressed.empty());
			}
		}

		if (blahblah.size() > 0) {
			std::vector<std::byte> bad_decompressed;
			bad_decompressed.resize(input.size() - 1);

			try {
				augs::decompress(augs::compress(state, input), bad_decompressed);
#if ENABLE_COMPRESSION
				REQUIRE(false);
#endif
			}
			catch(const augs::decompression_error&) { 
				REQUIRE(bad_decompressed.empty());
			}
		}
	}
}
#endif
