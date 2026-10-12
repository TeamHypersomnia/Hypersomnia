#pragma once
#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace augs {
	/*
		Standard base64 (RFC 4648, with "+/" and "=" padding), e.g. for images inside JSON.
	*/

	inline std::string to_base64(const std::byte* const data, const std::size_t size) {
		static constexpr char alphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

		auto out = std::string();
		out.reserve((size + 2) / 3 * 4);

		for (std::size_t i = 0; i < size; i += 3) {
			const auto b0 = static_cast<uint32_t>(data[i]);
			const auto b1 = i + 1 < size ? static_cast<uint32_t>(data[i + 1]) : 0u;
			const auto b2 = i + 2 < size ? static_cast<uint32_t>(data[i + 2]) : 0u;
			const auto triple = (b0 << 16) | (b1 << 8) | b2;

			out += alphabet[(triple >> 18) & 63];
			out += alphabet[(triple >> 12) & 63];
			out += i + 1 < size ? alphabet[(triple >> 6) & 63] : '=';
			out += i + 2 < size ? alphabet[triple & 63] : '=';
		}

		return out;
	}

	inline std::string to_base64(const std::vector<std::byte>& bytes) {
		return to_base64(bytes.data(), bytes.size());
	}

	/*
		Strict: only the alphabet and the padding at the end, a length divisible by 4.
	*/

	inline std::optional<std::vector<std::byte>> from_base64(const std::string_view text) {
		auto value_of = [](const char c) -> int {
			if (c >= 'A' && c <= 'Z') {
				return c - 'A';
			}

			if (c >= 'a' && c <= 'z') {
				return c - 'a' + 26;
			}

			if (c >= '0' && c <= '9') {
				return c - '0' + 52;
			}

			if (c == '+') {
				return 62;
			}

			if (c == '/') {
				return 63;
			}

			return -1;
		};

		if (text.size() % 4 != 0) {
			return std::nullopt;
		}

		auto out = std::vector<std::byte>();
		out.reserve(text.size() / 4 * 3);

		for (std::size_t i = 0; i < text.size(); i += 4) {
			const bool last = i + 4 == text.size();
			const auto padding = last ? (text[i + 3] == '=' ? (text[i + 2] == '=' ? 2 : 1) : 0) : 0;

			uint32_t quad = 0;

			for (std::size_t j = 0; j < 4; ++j) {
				const auto c = text[i + j];

				if (j >= 4 - static_cast<std::size_t>(padding)) {
					quad <<= 6;
					continue;
				}

				const auto v = value_of(c);

				if (v < 0) {
					return std::nullopt;
				}

				quad = (quad << 6) | static_cast<uint32_t>(v);
			}

			out.push_back(static_cast<std::byte>((quad >> 16) & 0xFF));

			if (padding < 2) {
				out.push_back(static_cast<std::byte>((quad >> 8) & 0xFF));
			}

			if (padding < 1) {
				out.push_back(static_cast<std::byte>(quad & 0xFF));
			}
		}

		return out;
	}
}
