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

	/*
		The key scheme, as it is named to players and operators.
	*/

	inline constexpr std::string_view key_scheme_name = "Ed25519";

	using ed25519_seed_type = std::array<uint8_t, 32>;
	using ed25519_public_key_type = std::array<uint8_t, 32>;
	using ed25519_signature_type = std::array<uint8_t, 64>;

	/*
		The secret key the way libsodium (and netcode) keeps it: the seed, then the public key.
	*/

	using ed25519_secret_key_type = std::array<uint8_t, 64>;

	std::optional<ed25519_seed_type> ed25519_generate_seed();

	/*
		Bytes from the same secure source as the seeds - false where there is none.
	*/

	bool secure_random_bytes(uint8_t* out, std::size_t count);
	std::optional<ed25519_public_key_type> ed25519_public_key(const ed25519_seed_type& seed);
	std::optional<ed25519_secret_key_type> ed25519_secret_key(const ed25519_seed_type& seed);

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
