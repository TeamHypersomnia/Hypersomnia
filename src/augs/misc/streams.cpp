#include "streams.h"
#include "enum_associative_array.h"

namespace augs {
	void write_object(augs::stream& ar, const augs::stream& storage) {
		ar.write(storage);
	}

	void stream::read(char* data, const size_t bytes) {
		if (!has_read_failed && read_pos + bytes <= size()) {
			memcpy(data, buf.data() + read_pos, bytes);
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

	char* stream::data() {
		return buf.data();
	}

	const char* stream::data() const {
		return buf.data();
	}

	char& stream::operator[](const size_t idx) {
		return data()[idx];
	}

	const char& stream::operator[](const size_t idx) const {
		return data()[idx];
	}
	
	std::string stream::to_string() const {
		return std::string(data(), data() + size());
	}

	size_t stream::capacity() const {
		return buf.size();
	}

	void stream::write(const char* const data, const size_t bytes) {
		if (write_pos + bytes > capacity()) {
			reserve((write_pos + bytes) * 2);
		}

		memcpy(buf.data() + write_pos, data, bytes);
		write_pos += bytes;
	}

	void stream::write(const augs::stream& s) {
		write(s.data(), s.size());
	}

	void output_stream_reserver::write(const char* const, const size_t bytes) {
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