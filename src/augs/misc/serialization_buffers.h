#pragma once
#include <vector>
#include "augs/readwrite/memory_stream_declaration.h"

namespace augs {
	std::vector<std::byte> make_compression_state();

	struct serialization_buffers {
		std::vector<std::byte> serialization;
		std::vector<std::byte> compressed;
		std::vector<std::byte> compression_state;

		serialization_buffers() : compression_state(make_compression_state()) {}

		template <class T = ref_memory_stream, class... Args>
		T make_serialization_stream(Args&&... args) {
			serialization.clear();
			return T(std::forward<Args>(args)..., serialization);
		}
	};
}
