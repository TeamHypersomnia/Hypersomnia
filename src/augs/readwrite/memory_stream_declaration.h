#pragma once
#include <vector>
#include <cstddef>

namespace augs {
	template <class B>
	class memory_stream_mixin;

	template <class B>
	class basic_ref_memory_stream;

	template <class B>
	class basic_memory_stream;

	using memory_stream = basic_memory_stream<std::vector<std::byte>>;

	using ref_memory_stream = basic_ref_memory_stream<std::vector<std::byte>>;
	using cref_memory_stream = basic_ref_memory_stream<const std::vector<std::byte>>;
}
