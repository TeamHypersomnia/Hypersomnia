#include "streams.h"

namespace augs {
	void stream::read(char* data, size_t bytes) {
		memcpy(data, buf.data() + pos, bytes);
		pos += bytes;
	}

	bool stream::operator==(const stream& b) const {
		return std::equal(buf.begin(), buf.end(), b.buf.begin(), b.buf.end());
	}

	size_t stream::size() const {
		return pos;
	}

	char* stream::data() {
		return buf.data();
	}

	const char* stream::data() const {
		return buf.data();
	}

	void stream::reset_pos() {
		pos = 0;
	}

	void stream::write(const char* data, size_t bytes) {
		memcpy(buf.data() + pos, data, bytes);
		pos += bytes;
	}

	void output_stream_reserver::write(const char*, size_t bytes) {
		size += bytes;
	}

	stream output_stream_reserver::make_stream() {
		stream reserved;
		reserved.reserve(size);
		return std::move(reserved);
	}

	void stream::reserve(size_t bytes) {
		reset_pos();
		buf.reserve(bytes);
	};
	
	void stream::reserve(const output_stream_reserver& r) {
		reserve(r.size);
	};
}