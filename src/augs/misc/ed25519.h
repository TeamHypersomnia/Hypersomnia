#pragma once
#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <string_view>

namespace augs {
	/*
		Ed25519 through libsodium. Every function returns std::nullopt or false
		on builds without networking or if libsodium fails.

		The private key is kept as its 32-byte seed, the form RFC 8032 defines it in.
	*/

	using ed25519_seed_type = std::array<uint8_t, 32>;
	using ed25519_public_key_type = std::array<uint8_t, 32>;
	using ed25519_signature_type = std::array<uint8_t, 64>;

	std::optional<ed25519_seed_type> ed25519_generate_seed();
	std::optional<ed25519_public_key_type> ed25519_public_key(const ed25519_seed_type& seed);

	std::optional<ed25519_signature_type> ed25519_sign(
		const ed25519_seed_type& seed,
		std::string_view message
	);

	bool ed25519_verify(
		const ed25519_public_key_type& public_key,
		std::string_view message,
		const ed25519_signature_type& signature
	);
}
