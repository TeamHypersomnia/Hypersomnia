#pragma once
#include "augs/ensure_rel.h"

namespace augs {
	struct pointer_to_buffer {
		std::byte* const buf;
		const std::size_t byte_count;

		auto data() {
			return buf;
		}

		const auto* data() const {
			return buf;
		}

		auto size() const {
			return byte_count;
		}

		void resize(std::size_t n) {
			/* Should never have to resize to a different number than byte_count! */
			ensure_eq(byte_count, n);
		}
	};

	struct cpointer_to_buffer {
		const std::byte* const buf;
		const std::size_t byte_count;

		const auto* data() const {
			return buf;
		}

		auto size() const {
			return byte_count;
		}
	};
}
