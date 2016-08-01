#include <bitset>
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

	void bits_stream::read_variable_length_index(unsigned short& idx) {
		std::bitset<4> index_bitcount;
		read(index_bitcount);

		auto bit_count = index_bitcount.to_ulong();

		std::bitset<16> decoded_index;
		read(decoded_index, bit_count);

		idx = decoded_index.to_ulong();
	}

	void bits_stream::write_variable_length_index(unsigned short idx) {
		if (idx == 0) {
			write(std::bitset<4>());
			return;
		}

		unsigned long mask = idx;
		unsigned long index;

		bool non_zero = _BitScanReverse(&index, mask);

		const unsigned bits_needed = index + 1;
		
		write(std::bitset<4>(bits_needed));
		write(std::bitset<16>(idx), bits_needed);
	}
}