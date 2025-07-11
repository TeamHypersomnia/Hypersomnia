#pragma once
#include <cstdint>
#include <vector>
#include <cstddef>
#include <string>

#include "augs/misc/constant_size_string.h"
#include "augs/readwrite/memory_stream_declaration.h"

namespace augs {
	using secure_hash_type = std::array<uint8_t, 32>;
	using hash_string_type = constant_size_string<64>;

	std::size_t crlf_to_lf(char*, std::size_t n);
	void crlf_to_lf(std::string&);
	void crlf_to_lf(std::vector<std::byte>&);

	template <class T>
	std::string crlf_to_lf_string(const T& bytes) {
		auto s = std::string(
			reinterpret_cast<const char*>(bytes.data()), 
			reinterpret_cast<const char*>(bytes.data() + bytes.size())
		);

		crlf_to_lf(s);

		return s;
	}

	hash_string_type to_hex_format(const secure_hash_type& hstr);
	secure_hash_type to_secure_hash_byte_format(const std::string& hex_string);

	secure_hash_type secure_hash(const std::byte* bytes, std::size_t n);

	template <class S>
	inline secure_hash_type secure_hash(const S& bytes) {
		return secure_hash(
			reinterpret_cast<const std::byte*>(bytes.data()),
			bytes.size()
		);
	}

	inline secure_hash_type secure_hash(const std::vector<std::byte>& bytes) {
		return secure_hash(bytes.data(), bytes.size());
	}
}

namespace std {
    template<>
    struct hash<augs::secure_hash_type> {
        size_t operator()(const augs::secure_hash_type& arr) const {
            size_t hash = 0;
            const size_t prime = 16777619;
            const size_t offset_basis = 2166136261U;

            for (const auto& element : arr) {
                hash = hash ^ element;
                hash = hash * prime;
            }

            return hash ^ (offset_basis * prime);
        }
    };
}
