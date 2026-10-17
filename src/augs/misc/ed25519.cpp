#include <algorithm>
#include <array>
#include "augs/misc/ed25519.h"

#if BUILD_NETWORKING
#include <sodium.h>
#endif

namespace augs {
#if BUILD_NETWORKING
	static bool sodium_ready() {
		static const bool ready = sodium_init() >= 0;
		return ready;
	}
#endif

	std::optional<ed25519_seed_type> ed25519_generate_seed() {
#if BUILD_NETWORKING
		if (!sodium_ready()) {
			return std::nullopt;
		}

		auto seed = ed25519_seed_type();
		randombytes_buf(seed.data(), seed.size());
		return seed;
#else
		return std::nullopt;
#endif
	}

	bool secure_random_bytes(uint8_t* const out, const std::size_t count) {
#if BUILD_NETWORKING
		if (!sodium_ready()) {
			return false;
		}

		randombytes_buf(out, count);
		return true;
#else
		(void)out;
		(void)count;
		return false;
#endif
	}

	std::optional<ed25519_secret_key_type> ed25519_secret_key(const ed25519_seed_type& seed) {
		const auto public_key = ed25519_public_key(seed);

		if (!public_key.has_value()) {
			return std::nullopt;
		}

		auto out = ed25519_secret_key_type();
		std::copy(seed.begin(), seed.end(), out.begin());
		std::copy(public_key->begin(), public_key->end(), out.begin() + seed.size());
		return out;
	}

	std::optional<ed25519_public_key_type> ed25519_public_key(const ed25519_seed_type& seed) {
#if BUILD_NETWORKING
		if (!sodium_ready()) {
			return std::nullopt;
		}

		auto out = ed25519_public_key_type();
		auto secret_key = std::array<uint8_t, crypto_sign_SECRETKEYBYTES>();

		const auto result = crypto_sign_seed_keypair(out.data(), secret_key.data(), seed.data());
		sodium_memzero(secret_key.data(), secret_key.size());

		if (result != 0) {
			return std::nullopt;
		}

		return out;
#else
		(void)seed;
		return std::nullopt;
#endif
	}

	std::optional<ed25519_signature_type> ed25519_sign(
		const ed25519_seed_type& seed,
		const std::string_view message
	) {
#if BUILD_NETWORKING
		if (!sodium_ready()) {
			return std::nullopt;
		}

		auto public_key = ed25519_public_key_type();
		auto secret_key = std::array<uint8_t, crypto_sign_SECRETKEYBYTES>();
		auto out = ed25519_signature_type();

		const auto* const data = reinterpret_cast<const unsigned char*>(message.data());

		const auto result =
			crypto_sign_seed_keypair(public_key.data(), secret_key.data(), seed.data()) == 0
			&& crypto_sign_detached(out.data(), nullptr, data, message.size(), secret_key.data()) == 0
		;

		sodium_memzero(secret_key.data(), secret_key.size());

		if (!result) {
			return std::nullopt;
		}

		return out;
#else
		(void)seed;
		(void)message;
		return std::nullopt;
#endif
	}

	bool ed25519_verify(
		const ed25519_public_key_type& public_key,
		const std::string_view message,
		const ed25519_signature_type& signature
	) {
#if BUILD_NETWORKING
		if (!sodium_ready()) {
			return false;
		}

		/*
			libsodium rejects the public keys and R of small order - for which a fixed signature
			would verify with any message - and the non-canonical encodings.
		*/

		const auto* const data = reinterpret_cast<const unsigned char*>(message.data());
		return crypto_sign_verify_detached(signature.data(), data, message.size(), public_key.data()) == 0;
#else
		(void)public_key;
		(void)message;
		(void)signature;
		return false;
#endif
	}
}

#if BUILD_UNIT_TESTS && BUILD_NETWORKING
#include <string>
#include <Catch/single_include/catch2/catch.hpp>

template <std::size_t N>
static std::array<uint8_t, N> from_hex(const std::string& hex) {
	auto out = std::array<uint8_t, N>();

	for (std::size_t i = 0; i < N; ++i) {
		out[i] = static_cast<uint8_t>(std::stoi(hex.substr(i * 2, 2), nullptr, 16));
	}

	return out;
}

TEST_CASE("Ed25519 RFC 8032 test 1") {
	const auto seed = from_hex<32>("9d61b19deffd5a60ba844af492ec2cc44449c5697b326919703bac031cae7f60");
	const auto public_key = from_hex<32>("d75a980182b10ab7d54bfed3c964073a0ee172f3daa62325af021a68f707511a");
	const auto signature = from_hex<64>(
		"e5564300c360ac729086e2cc806e828a84877f1eb8e5d974d873e065224901555fb8821590a33bacc61e39701cf9b46bd25bf5f0595bbe24655141438e7a100b"
	);

	REQUIRE(augs::ed25519_public_key(seed) == public_key);
	REQUIRE(augs::ed25519_sign(seed, "") == signature);
	REQUIRE(augs::ed25519_verify(public_key, "", signature));

	auto tampered = signature;
	tampered[0] ^= 1;

	REQUIRE(!augs::ed25519_verify(public_key, "", tampered));
	REQUIRE(!augs::ed25519_verify(public_key, "x", signature));
}

TEST_CASE("Ed25519 rejects the public keys of small order") {
	auto small_order_key = augs::ed25519_public_key_type();
	small_order_key[0] = 1;

	auto signature = augs::ed25519_signature_type();
	signature[0] = 1;

	REQUIRE(!augs::ed25519_verify(small_order_key, "hypersomnia-auth-v1 anything", signature));
	REQUIRE(!augs::ed25519_verify(small_order_key, "", signature));
	REQUIRE(!augs::ed25519_verify(augs::ed25519_public_key_type(), "", augs::ed25519_signature_type()));

	/* The same point with the sign bit set */
	small_order_key[31] = 0x80;
	REQUIRE(!augs::ed25519_verify(small_order_key, "", signature));
}

TEST_CASE("Ed25519 generated keys sign and verify") {
	const auto seed = augs::ed25519_generate_seed();
	REQUIRE(seed.has_value());

	const auto public_key = augs::ed25519_public_key(*seed);
	REQUIRE(public_key.has_value());

	const auto message = std::string("hypersomnia-auth-v1 hs_0123@de");
	const auto signature = augs::ed25519_sign(*seed, message);

	REQUIRE(signature.has_value());
	REQUIRE(augs::ed25519_verify(*public_key, message, *signature));
	REQUIRE(!augs::ed25519_verify(*public_key, "hypersomnia-auth-v1 hs_0123@pl", *signature));
}

#endif
