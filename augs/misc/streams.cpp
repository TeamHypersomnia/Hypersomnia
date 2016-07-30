#include "streams.h"

namespace augs {
	void input_stream::read(char* data, size_t bytes) {
		memcpy(data, buf.data() + pos, bytes);
		pos += bytes;
	}

	void output_stream::reserve(size_t bytes) {
		buf.reserve(bytes);
	};
	
	void output_stream::write(const char* data, size_t bytes) {
		buf.insert(buf.end(), data, data + bytes);
		pos += bytes;
	}

	bool output_stream::operator==(const output_stream& b) const {
		return std::equal(buf.begin(), buf.end(), b.buf.begin(), b.buf.end());
	}

	bool output_stream_unsafe::operator==(const output_stream_unsafe& b) const {
		return std::equal(buf.begin(), buf.end(), b.buf.begin(), b.buf.end());
	}

	void output_stream_unsafe::write(const char* data, size_t bytes) {
		memcpy(buf.data() + pos, data, bytes);
		pos += bytes;
	}

	void output_stream_reserver::write(const char*, size_t bytes) {
		size += bytes;
	}

	output_stream_unsafe output_stream_reserver::make_stream() {
		output_stream_unsafe reserved;
		reserved.reserve(size);
		return std::move(reserved);
	}

	void output_stream_unsafe::reserve(size_t bytes) {
		buf.resize(bytes);
	};
}