#pragma once

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
