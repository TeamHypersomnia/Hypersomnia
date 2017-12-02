#include <cstring>
#include <algorithm>
#include "augs/readwrite/memory_stream.h"

namespace augs {
	memory_stream::memory_stream(std::vector<std::byte>&& new_buffer)
		: buffer(std::move(new_buffer))
	{
		set_write_pos(buffer.size());
	}

	memory_stream& memory_stream::operator=(std::vector<std::byte>&& new_buffer) {
		buffer = std::move(new_buffer);
		set_write_pos(buffer.size());
		set_read_pos(0);
		return *this;
	}

	void memory_stream::read(std::byte* data, const std::size_t bytes) {
		if (read_pos + bytes > size()) {
			throw stream_read_error(
				"Failed to read bytes: %x-%x (size: %x)", 
				read_pos, 
				read_pos + bytes, 
				size()
			);
		}
		
		std::memcpy(data, buffer.data() + read_pos, bytes);
		read_pos += bytes;
	}
	
	std::size_t memory_stream::mismatch(const memory_stream& b) const {
		return std::mismatch(data(), data() + size(), b.data()).first - data();
	}

	bool memory_stream::operator==(const memory_stream& b) const {
		return std::equal(data(), data() + size(), b.data());
	}

	bool memory_stream::operator!=(const memory_stream& b) const {
		return !operator==(b);
	}

	std::byte* memory_stream::data() {
		return buffer.data();
	}

	const std::byte* memory_stream::data() const {
		return buffer.data();
	}

	std::byte& memory_stream::operator[](const size_t idx) {
		return data()[idx];
	}

	const std::byte& memory_stream::operator[](const size_t idx) const {
		return data()[idx];
	}
	
	size_t memory_stream::capacity() const {
		return buffer.size();
	}

	void memory_stream::write(const std::byte* const data, const size_t bytes) {
#if 0
		if (write_pos + bytes >= 2663) {
			__debugbreak();
		}
#endif
		if (write_pos + bytes > capacity()) {
			reserve((write_pos + bytes) * 2);
		}

		std::memcpy(buffer.data() + write_pos, data, bytes);
		write_pos += bytes;
	}

	void memory_stream::write(const augs::memory_stream& s) {
		write(s.data(), s.size());
	}

	void byte_counter_stream::write(const std::byte* const, const std::size_t bytes) {
		write_pos += bytes;
	}

	memory_stream byte_counter_stream::create_reserved_stream() {
		memory_stream reserved;
		reserved.reserve(write_pos);
		return reserved;
	}

	void memory_stream::reserve(const std::size_t bytes) {
		buffer.resize(bytes);
	};
	
	void memory_stream::reserve(const byte_counter_stream& r) {
		reserve(r.size());
	};
}