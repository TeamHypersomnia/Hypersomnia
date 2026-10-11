#pragma once
#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <string_view>
#include <vector>

namespace augs {
	/*
		Strict: only hex digits, an even count, no signs, spaces or prefixes.
		lowercase_only for canonical forms, e.g. an account id - "key_AB.." must not name the same key as "key_ab..".
	*/

	inline int hex_nibble(const char ch, const bool lowercase_only = false) {
		if (ch >= '0' && ch <= '9') {
			return ch - '0';
		}

		if (ch >= 'a' && ch <= 'f') {
			return ch - 'a' + 10;
		}

		if (!lowercase_only && ch >= 'A' && ch <= 'F') {
			return ch - 'A' + 10;
		}

		return -1;
	}

	inline std::optional<std::vector<uint8_t>> hex_to_bytes(const std::string_view hex, const bool lowercase_only = false) {
		if (hex.size() % 2 != 0) {
			return std::nullopt;
		}

		auto out = std::vector<uint8_t>();
		out.reserve(hex.size() / 2);

		for (std::size_t i = 0; i < hex.size(); i += 2) {
			const auto hi = hex_nibble(hex[i], lowercase_only);
			const auto lo = hex_nibble(hex[i + 1], lowercase_only);

			if (hi < 0 || lo < 0) {
				return std::nullopt;
			}

			out.push_back(static_cast<uint8_t>(hi * 16 + lo));
		}

		return out;
	}

	template <std::size_t N>
	std::optional<std::array<uint8_t, N>> hex_to_array(const std::string_view hex, const bool lowercase_only = false) {
		if (hex.size() != N * 2) {
			return std::nullopt;
		}

		const auto bytes = hex_to_bytes(hex, lowercase_only);

		if (!bytes.has_value()) {
			return std::nullopt;
		}

		auto out = std::array<uint8_t, N>();

		for (std::size_t i = 0; i < N; ++i) {
			out[i] = (*bytes)[i];
		}

		return out;
	}
}
