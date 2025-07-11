#pragma once
#include <cstddef>
#include <cstdint>
#include <string>
#include <array>

std::string get_hex_representation(const uint8_t *Bytes, const size_t Length);
std::string get_hex_representation(const std::byte *Bytes, const size_t Length);

template <std::size_t N>
std::string get_hex_representation(const std::array<uint8_t, N> bytes) {
	return get_hex_representation(bytes.data(), bytes.size());
}
