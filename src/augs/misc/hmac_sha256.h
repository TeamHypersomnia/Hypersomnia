#pragma once
#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <string_view>

namespace augs {
	using hmac_sha256_type = std::array<uint8_t, 32>;

	/*
		HMAC-SHA256 through OpenSSL.
		std::nullopt on builds without OpenSSL (e.g. the web) or if OpenSSL fails.
	*/

	std::optional<hmac_sha256_type> hmac_sha256(
		const std::byte* key,
		std::size_t key_size,
		const std::byte* message,
		std::size_t message_size
	);

	template <class Key, class Message>
	std::optional<hmac_sha256_type> hmac_sha256(const Key& key, const Message& message) {
		return hmac_sha256(
			reinterpret_cast<const std::byte*>(key.data()),
			key.size(),
			reinterpret_cast<const std::byte*>(message.data()),
			message.size()
		);
	}

	/*
		Compares in time independent of where the first difference is,
		so that a MAC can not be guessed byte by byte from response timings.
	*/

	template <std::size_t N>
	bool constant_time_equal(const std::array<uint8_t, N>& a, const std::array<uint8_t, N>& b) {
		volatile uint8_t difference = 0;

		for (std::size_t i = 0; i < N; ++i) {
			difference = difference | static_cast<uint8_t>(a[i] ^ b[i]);
		}

		return difference == 0;
	}
}
