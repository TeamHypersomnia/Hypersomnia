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
	std::string secure_hash(const std::byte* bytes, std::size_t n) {
		blake3_hasher hasher;
		blake3_hasher_init(&hasher);

		blake3_hasher_update(&hasher, bytes, n);

		uint8_t output[BLAKE3_OUT_LEN];
		blake3_hasher_finalize(&hasher, output, BLAKE3_OUT_LEN);

		return get_hex_representation(output, BLAKE3_OUT_LEN);
	}
}
