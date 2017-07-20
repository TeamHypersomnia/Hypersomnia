#include "streams.h"
#include "enum_associative_array.h"

namespace augs {
	void write_object(augs::stream& ar, const augs::stream& storage) {
		ar.write(storage);
	}

	void stream::read(std::byte* data, const size_t bytes) {
		if (!has_read_failed && read_pos + bytes <= size()) {
			std::memcpy(data, buf.data() + read_pos, bytes);
			read_pos += bytes;
		}
		else {
			has_read_failed = true;
		}

		ensure(!has_read_failed);
	}
	
	std::string stream::format_as_uchars() const {
		const auto first = reinterpret_cast<const unsigned char*>(data());

		std::string output;

		for (size_t i = 0; i < size(); ++i) {
			output += std::to_string(static_cast<int>(first[i])) + " ";
		}

		if (output.size() > 0) {
			output.erase(output.end() - 1);
		}

		return output;
	}

	std::size_t stream::mismatch(const stream& b) const {
		return std::mismatch(data(), data() + size(), b.data()).first - data();
	}

	bool stream::operator==(const stream& b) const {
		return std::equal(data(), data() + size(), b.data());
	}

	bool stream::operator!=(const stream& b) const {
		return !operator==(b);
	}

	std::byte* stream::data() {
		return buf.data();
	}

	const std::byte* stream::data() const {
		return buf.data();
	}

	std::byte& stream::operator[](const size_t idx) {
		return data()[idx];
	}

	const std::byte& stream::operator[](const size_t idx) const {
		return data()[idx];
	}
	
	std::string stream::to_string() const {
		return std::string(data(), data() + size());
	}

	size_t stream::capacity() const {
		return buf.size();
	}

	void stream::write(const std::byte* const data, const size_t bytes) {
#if 0
		if (write_pos + bytes >= 2663) {
			__debugbreak();
		}
#endif
		if (write_pos + bytes > capacity()) {
			reserve((write_pos + bytes) * 2);
		}

		std::memcpy(buf.data() + write_pos, data, bytes);
		write_pos += bytes;
	}

	void stream::write(const augs::stream& s) {
		write(s.data(), s.size());
	}

	void output_stream_reserver::write(const std::byte* const, const size_t bytes) {
		write_pos += bytes;
	}

	stream output_stream_reserver::make_stream() {
		stream reserved;
		reserved.reserve(write_pos);
		return reserved;
	}

	void stream::reserve(const size_t bytes) {
		buf.resize(bytes);
	};
	
	void stream::reserve(const output_stream_reserver& r) {
		reserve(r.size());
	};
}