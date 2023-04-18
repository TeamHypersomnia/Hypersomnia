#include <sstream>
#include <iomanip>

#include "augs/misc/secure_hash.h"
#include "3rdparty/blake3/blake3.h"

std::string get_hex_representation(const unsigned char *Bytes, const size_t Length) {
	std::ostringstream os;
	os.fill('0');
	os<<std::hex;
	for(const unsigned char *ptr = Bytes; ptr < Bytes+Length; ++ptr) {
		os<<std::setw(2)<<(unsigned int)*ptr;
	}
	return os.str();
}

namespace augs {
	void crlf_to_lf(std::string& input) {
		std::size_t read_idx = 0;
		std::size_t write_idx = 0;

		while (read_idx < input.size()) {
			if (input[read_idx] == '\r' && read_idx + 1 < input.size() && input[read_idx + 1] == '\n') {
				input[write_idx++] = '\n';
				read_idx += 2;  // Skip the '\r' and the '\n'
			} else {
				input[write_idx++] = input[read_idx++];
			}
		}

		input.resize(write_idx);  // Truncate the string to the new size
	}

	hash_string_type to_hex_format(const secure_hash_type& hstr) {
		static_assert(hash_string_type().max_size() == BLAKE3_OUT_LEN * 2);

		return get_hex_representation(hstr.data(), hstr.size());
	}

	secure_hash_type secure_hash(const std::byte* bytes, std::size_t n) {
		static_assert(secure_hash_type().size() == BLAKE3_OUT_LEN);

		blake3_hasher hasher;
		blake3_hasher_init(&hasher);

		blake3_hasher_update(&hasher, bytes, n);

		secure_hash_type output;
		blake3_hasher_finalize(&hasher, output.data(), output.size());

		return output;
	}
}
