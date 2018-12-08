#pragma once
#include <vector>

namespace augs {
	std::vector<std::byte> make_compressor_state();

	std::vector<std::byte> compress(
		std::vector<std::byte>& state,
		const std::vector<std::byte>& input
	);

	void compress(
		std::vector<std::byte>& state,
		const std::vector<std::byte>& input,
		std::vector<std::byte>& output
	);

	std::vector<std::byte> decompress(
		const std::vector<std::byte>& input,
		std::size_t original_size
	);

	void decompress(
		const std::vector<std::byte>& input,
		std::vector<std::byte>& output
	);
}
