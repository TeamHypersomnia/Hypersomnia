#pragma once
#include <cstddef>

namespace augs {
	inline auto file_to_bytes(const path_type& path, std::vector<std::byte>& output) {
		auto file = with_exceptions<std::ifstream>();
		file.open(path, std::ios::binary | std::ios::ate);

		const std::streamsize size = file.tellg();
		file.seekg(0, std::ios::beg);

		resize_no_init(output, static_cast<std::size_t>(size));
		file.read(reinterpret_cast<byte_type_for_t<std::ifstream>*>(output.data()), size);
		return output;
	}

	inline auto file_to_bytes(const path_type& path) {
		std::vector<std::byte> output;
		file_to_bytes(path, output);
		return output;
	}
}
