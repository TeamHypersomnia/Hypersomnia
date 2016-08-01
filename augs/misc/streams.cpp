#include "streams.h"

namespace augs {
	void stream::read(char* data, size_t bytes) {
		memcpy(data, buf.data() + pos, bytes);
		pos += bytes;
	}

	bool stream::operator==(const stream& b) const {
		return std::equal(data(), data() + size(), b.data(), b.data() + size());
	}

	char* stream::data() {
		return buf.data();
	}

	const char* stream::data() const {
		return buf.data();
	}

	void stream::write(const char* data, size_t bytes) {
		memcpy(buf.data() + pos, data, bytes);
		pos += bytes;
	}

	void output_stream_reserver::write(const char*, size_t bytes) {
		pos += bytes;
	}

	stream output_stream_reserver::make_stream() {
		stream reserved;
		reserved.reserve(pos);
		return std::move(reserved);
	}

	void stream::reserve(size_t bytes) {
		reset_pos();
		buf.resize(bytes);
	};
	
	void stream::reserve(const output_stream_reserver& r) {
		reserve(r.size());
	};
}