#include "augs/misc/compress.h"
#include "3rdparty/lz4/lz4.c"

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
		const auto size_bound = LZ4_compressBound(input.size());
		output.resize(size_bound);

		const auto sz = LZ4_compress_fast_extState(
			reinterpret_cast<void*>(state.data()), 
			reinterpret_cast<const char*>(input.data()), 
			reinterpret_cast<char*>(output.data()), 
			input.size(),
			size_bound,
			1
		);

		output.resize(sz);
	}

	std::vector<std::byte> decompress(
		const std::vector<std::byte>& input,
		std::size_t original_size
	) {
		std::vector<std::byte> output;
		output.resize(original_size);

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
		const auto sz = LZ4_decompress_safe(
			reinterpret_cast<const char*>(input), 
			reinterpret_cast<char*>(output.data()), 
			byte_count,
			output.size()
		);

		if (sz < 0) {
			output.clear();
			return;
		}

		if (output.size() != static_cast<std::size_t>(sz)) {
			output.clear();
		}
	}
}

#if BUILD_UNIT_TESTS
#include <Catch/single_include/catch2/catch.hpp>
#include "augs/log.h"

TEST_CASE("Ca CompressionDecompression") {
	const auto tests = {
		"",
		"jsdfhsdkj fhdslkjfh37824y239874y298734y9382ejhfdffgsdfgsdfgsfdgdsfglkjdfsjglskdfjglksdfjgsdfgjhksdfj ghsdjfghsdjkfghsjfkdghskdfgkfsdjghkj",
		"jsdfhsdkj 8923749087sdewlak fidsuj 89r2 jklsdfjj 89023jfoui23jfjhglkddjgh87934ylalksja;lsdalf89kl43jvgfh7823dsakjhd78t3kjhsd78"
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

		REQUIRE(compressed != input);

		{
			const auto decompressed = augs::decompress(augs::compress(state, input), input.size());

			REQUIRE(decompressed == input);
		}

		{
			const auto bad_decompressed = augs::decompress(augs::compress(state, input), input.size() + 1);

			REQUIRE(bad_decompressed.empty());
		}

		if (blahblah.size() > 0) {
			const auto bad_decompressed = augs::decompress(augs::compress(state, input), input.size() - 1);

			REQUIRE(bad_decompressed.empty());
		}
	}
}
#endif
