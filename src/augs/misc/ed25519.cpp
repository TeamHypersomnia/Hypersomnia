#include <memory>
#include "augs/misc/ed25519.h"

#if BUILD_OPENSSL
#include <openssl/evp.h>
#include <openssl/rand.h>
#endif

namespace augs {
#if BUILD_OPENSSL
	struct evp_pkey_deleter {
		void operator()(EVP_PKEY* const p) const {
			EVP_PKEY_free(p);
		}
	};

	struct evp_md_ctx_deleter {
		void operator()(EVP_MD_CTX* const p) const {
			EVP_MD_CTX_free(p);
		}
	};

	using evp_pkey_ptr = std::unique_ptr<EVP_PKEY, evp_pkey_deleter>;
	using evp_md_ctx_ptr = std::unique_ptr<EVP_MD_CTX, evp_md_ctx_deleter>;

	static evp_pkey_ptr private_key_from(const ed25519_seed_type& seed) {
		return evp_pkey_ptr(EVP_PKEY_new_raw_private_key(EVP_PKEY_ED25519, nullptr, seed.data(), seed.size()));
	}
#endif

	std::optional<ed25519_seed_type> ed25519_generate_seed() {
#if BUILD_OPENSSL
		auto seed = ed25519_seed_type();

		if (RAND_bytes(seed.data(), static_cast<int>(seed.size())) != 1) {
			return std::nullopt;
		}

		return seed;
#else
		return std::nullopt;
#endif
	}

	std::optional<ed25519_public_key_type> ed25519_public_key(const ed25519_seed_type& seed) {
#if BUILD_OPENSSL
		const auto key = private_key_from(seed);

		if (key == nullptr) {
			return std::nullopt;
		}

		auto out = ed25519_public_key_type();
		auto length = out.size();

		if (EVP_PKEY_get_raw_public_key(key.get(), out.data(), &length) != 1 || length != out.size()) {
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
#if BUILD_OPENSSL
		const auto key = private_key_from(seed);
		const auto ctx = evp_md_ctx_ptr(EVP_MD_CTX_new());

		if (key == nullptr || ctx == nullptr) {
			return std::nullopt;
		}

		/*
			Ed25519 hashes the message itself, so no digest is given.
		*/

		if (EVP_DigestSignInit(ctx.get(), nullptr, nullptr, nullptr, key.get()) != 1) {
			return std::nullopt;
		}

		auto out = ed25519_signature_type();
		auto length = out.size();

		const auto* const data = reinterpret_cast<const unsigned char*>(message.data());

		if (EVP_DigestSign(ctx.get(), out.data(), &length, data, message.size()) != 1 || length != out.size()) {
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
#if BUILD_OPENSSL
		const auto key = evp_pkey_ptr(EVP_PKEY_new_raw_public_key(EVP_PKEY_ED25519, nullptr, public_key.data(), public_key.size()));
		const auto ctx = evp_md_ctx_ptr(EVP_MD_CTX_new());

		if (key == nullptr || ctx == nullptr) {
			return false;
		}

		if (EVP_DigestVerifyInit(ctx.get(), nullptr, nullptr, nullptr, key.get()) != 1) {
			return false;
		}

		const auto* const data = reinterpret_cast<const unsigned char*>(message.data());

		return EVP_DigestVerify(ctx.get(), signature.data(), signature.size(), data, message.size()) == 1;
#else
		(void)public_key;
		(void)message;
		(void)signature;
		return false;
#endif
	}
}

#if BUILD_UNIT_TESTS && BUILD_OPENSSL
#include <string>
#include <Catch/single_include/catch2/catch.hpp>
#include "augs/misc/hmac_sha256.h"

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

TEST_CASE("HMAC-SHA256 RFC 4231 test case 2") {
	const auto mac = augs::hmac_sha256(std::string("Jefe"), std::string("what do ya want for nothing?"));

	REQUIRE(mac.has_value());
	REQUIRE(*mac == from_hex<32>("5bdcc146bf60754e6a042426089575c75a003f089d2739839dec58b964ec3843"));
	REQUIRE(augs::constant_time_equal(*mac, *mac));
}
#endif
