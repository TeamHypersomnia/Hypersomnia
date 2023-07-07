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

augs::hash_string_type get_hex_representation(const augs::secure_hash_type& bytes) {
	static_assert(augs::hash_string_type().max_size() == augs::secure_hash_type().size() * 2);

	augs::hash_string_type hex_string;
	hex_string.resize_no_init(bytes.size() * 2);

	std::size_t i = 0;

	const char* hex_chars = "0123456789abcdef";

	for (const auto& byte : bytes) {
		const uint8_t high_nibble = (byte >> 4) & 0x0F;
		const uint8_t low_nibble = byte & 0x0F;

		hex_string[i++] = hex_chars[high_nibble];
		hex_string[i++] = hex_chars[low_nibble];
	}

	return hex_string;
}

namespace augs {
	std::size_t crlf_to_lf(char* input, std::size_t n) {
		std::size_t read_idx = 0;
		std::size_t write_idx = 0;

		while (read_idx < n) {
			if (input[read_idx] == '\r' && read_idx + 1 < n && input[read_idx + 1] == '\n') {
				input[write_idx++] = '\n';
				read_idx += 2;  // Skip the '\r' and the '\n'
			} else {
				input[write_idx++] = input[read_idx++];
			}
		}

		return write_idx;
	}

	void crlf_to_lf(std::string& input) {
		input.resize(crlf_to_lf(input.data(), input.size()));
	}

	void crlf_to_lf(std::vector<std::byte>& input) {
		input.resize(crlf_to_lf(reinterpret_cast<char*>(input.data()), input.size()));
	}

	secure_hash_type to_secure_hash_byte_format(const std::string& hex_string) {
		std::array<uint8_t, 32> bytes = {0};

		if (hex_string.length() != 64) {
			return bytes;
		}

		for (size_t i = 0; i < 64; i += 2) {
			const uint8_t high_nibble = hex_string[i] <= '9' ? hex_string[i] - '0' : hex_string[i] - 'a' + 10;
			const uint8_t low_nibble = hex_string[i + 1] <= '9' ? hex_string[i + 1] - '0' : hex_string[i + 1] - 'a' + 10;

			bytes[i / 2] = (high_nibble << 4) | low_nibble;
		}

		return bytes;
	}

	hash_string_type to_hex_format(const secure_hash_type& hstr) {
		static_assert(hash_string_type().max_size() == BLAKE3_OUT_LEN * 2);

		return get_hex_representation(hstr);
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
