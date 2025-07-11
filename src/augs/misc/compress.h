#pragma once
#include <cstddef>
#include <vector>
#include "augs/templates/exception_templates.h"

namespace augs {
	struct decompression_error : error_with_typesafe_sprintf {
		using error_with_typesafe_sprintf::error_with_typesafe_sprintf;
	};

	std::vector<std::byte> make_compression_state();

	std::vector<std::byte> compress(
		std::vector<std::byte>& state,
		const std::vector<std::byte>& input
	);

	void compress(
		std::vector<std::byte>& state,
		const std::vector<std::byte>& input,
		std::vector<std::byte>& output
	);

	void compress(
		std::vector<std::byte>& state,
		const std::byte* input,
		const std::size_t input_size,
		std::vector<std::byte>& output
	);

	std::vector<std::byte> decompress(
		const std::vector<std::byte>& input,
		std::size_t uncompressed_size
	);

	void decompress(
		const std::byte* input,
		std::size_t byte_count,
		std::vector<std::byte>& output
	);

	void decompress(
		const std::vector<std::byte>& input,
		std::vector<std::byte>& output
	);
}
