#pragma once
#include <vector>
#include <cstddef>

namespace augs {
	template <class B>
	class memory_stream_mixin;

	template <class B>
	class basic_ref_memory_stream;

	class memory_stream;

	using ref_memory_stream = basic_ref_memory_stream<std::vector<std::byte>>;
	using cref_memory_stream = basic_ref_memory_stream<const std::vector<std::byte>>;
}
