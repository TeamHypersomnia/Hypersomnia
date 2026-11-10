#pragma once
#include <chrono>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <limits>
#include <string>
#include <vector>
#include "augs/filesystem/file.h"
#include "augs/misc/compress.h"
#include "augs/readwrite/byte_file.h"

namespace augs {
	constexpr uint64_t lz4_max_expansion_ratio_v = 256;

	inline void bytes_to_lz4_file_atomic(const std::vector<std::byte>& bytes, const path_type& path) {
		const auto uncompressed_size = static_cast<uint64_t>(bytes.size());

		auto file_bytes = std::vector<std::byte>(sizeof(uncompressed_size));
		std::memcpy(file_bytes.data(), std::addressof(uncompressed_size), sizeof(uncompressed_size));

		auto state = augs::make_compression_state();
		augs::compress(state, bytes, file_bytes);

		auto tmp = path;
		tmp += ".tmp-" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count());

		augs::bytes_to_file(file_bytes, tmp);
		std::filesystem::rename(tmp, path);
	}

	inline std::vector<std::byte> lz4_file_to_bytes(const path_type& path) {
		const auto file_bytes = augs::file_to_bytes(path);
		auto uncompressed_size = uint64_t(0);

		if (file_bytes.size() < sizeof(uncompressed_size)) {
			throw decompression_error("%x is too short for an lz4 file.", path);
		}

		std::memcpy(std::addressof(uncompressed_size), file_bytes.data(), sizeof(uncompressed_size));

		const auto compressed_size = file_bytes.size() - sizeof(uncompressed_size);

		const bool larger_than_decompress_takes = uncompressed_size > static_cast<uint64_t>(std::numeric_limits<int>::max());

		if (larger_than_decompress_takes || uncompressed_size > (compressed_size + 1) * lz4_max_expansion_ratio_v) {
			throw decompression_error("%x claims %x bytes after decompression from only %x.", path, uncompressed_size, compressed_size);
		}

		auto output = std::vector<std::byte>(static_cast<std::size_t>(uncompressed_size));
		augs::decompress(file_bytes.data() + sizeof(uncompressed_size), compressed_size, output);

		return output;
	}
}
