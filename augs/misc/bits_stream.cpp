#include "bits_stream.h"

namespace augs {
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