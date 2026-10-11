#include <cstring>
#if BUILD_NETWORKING
#include <sodium.h>
#endif
#include "augs/misc/secure_hash.h"
#include "augs/string/hex.h"
#include "application/network/identity_cert.h"

namespace identity_cert {
	static void write_i64(std::vector<std::byte>& out, const int64_t value) {
		const auto u = static_cast<uint64_t>(value);

		for (int i = 0; i < 8; ++i) {
			out.push_back(static_cast<std::byte>((u >> (8 * i)) & 0xff));
		}
	}

	static int64_t read_i64(const std::byte* const p) {
		uint64_t u = 0;

		for (int i = 0; i < 8; ++i) {
			u |= static_cast<uint64_t>(static_cast<uint8_t>(p[i])) << (8 * i);
		}

		return static_cast<int64_t>(u);
	}

	static std::vector<std::byte> unsigned_bytes(const cert& c) {
		auto out = std::vector<std::byte>();

		out.push_back(static_cast<std::byte>(format_version));
		out.push_back(static_cast<std::byte>(c.account.size()));

		for (const auto ch : c.account) {
			out.push_back(static_cast<std::byte>(ch));
		}

		for (const auto b : c.session_public_key) {
			out.push_back(static_cast<std::byte>(b));
		}

		write_i64(out, c.not_before);
		write_i64(out, c.not_after);

		return out;
	}

	std::vector<std::byte> to_bytes(const cert& c) {
		auto out = unsigned_bytes(c);

		for (const auto b : c.signature) {
			out.push_back(static_cast<std::byte>(b));
		}

		return out;
	}

	std::optional<cert> from_bytes(const std::byte* const data, const std::size_t size) {
		if (data == nullptr || size < 2) {
			return std::nullopt;
		}

		if (static_cast<uint8_t>(data[0]) != format_version) {
			return std::nullopt;
		}

		const auto account_length = static_cast<std::size_t>(static_cast<uint8_t>(data[1]));

		if (account_length == 0 || account_length > max_account_length) {
			return std::nullopt;
		}

		const auto expected_size = 2 + account_length + 32 + 8 + 8 + 64;

		if (size != expected_size) {
			return std::nullopt;
		}

		auto out = cert();
		const auto* p = data + 2;

		out.account.assign(reinterpret_cast<const char*>(p), account_length);
		p += account_length;

		std::memcpy(out.session_public_key.data(), p, 32);
		p += 32;

		out.not_before = read_i64(p);
		p += 8;

		out.not_after = read_i64(p);
		p += 8;

		std::memcpy(out.signature.data(), p, 64);

		return out;
	}

	std::string signed_message(const cert& c) {
		const auto bytes = unsigned_bytes(c);

		auto out = std::string(cert_tag);
		out.append(reinterpret_cast<const char*>(bytes.data()), bytes.size());

		return out;
	}

	std::string key_account(const augs::ed25519_public_key_type& account_public_key) {
		return "key_" + augs::bytes_to_hex(account_public_key);
	}

	std::optional<cert> make_self_signed(
		const augs::ed25519_seed_type& account_seed,
		const augs::ed25519_public_key_type& session_public_key
	) {
		const auto account_public_key = augs::ed25519_public_key(account_seed);

		if (!account_public_key.has_value()) {
			return std::nullopt;
		}

		auto out = cert();
		out.account = key_account(*account_public_key);
		out.session_public_key = session_public_key;

		const auto signature = augs::ed25519_sign(account_seed, signed_message(out));

		if (!signature.has_value()) {
			return std::nullopt;
		}

		out.signature = *signature;
		return out;
	}

	std::string cert_request_message(const augs::ed25519_public_key_type& session_public_key) {
		auto out = std::string(cert_request_tag);
		out.append(reinterpret_cast<const char*>(session_public_key.data()), session_public_key.size());
		return out;
	}

	std::string steam_ticket_identity(const augs::ed25519_public_key_type& session_public_key) {
#if BUILD_NETWORKING
		auto message = std::string("hs-steam-v1");
		message.append(reinterpret_cast<const char*>(session_public_key.data()), session_public_key.size());

		auto hash = std::array<uint8_t, crypto_hash_sha256_BYTES>();
		crypto_hash_sha256(hash.data(), reinterpret_cast<const unsigned char*>(message.data()), message.size());

		return augs::bytes_to_hex(std::vector<uint8_t>(hash.begin(), hash.begin() + 15));
#else
		(void)session_public_key;
		return "";
#endif
	}

	static std::optional<augs::ed25519_public_key_type> key_of_key_account(const std::string& account) {
		static constexpr std::string_view prefix = "key_";

		if (account.size() != prefix.size() + 64 || account.compare(0, prefix.size(), prefix) != 0) {
			return std::nullopt;
		}

		return augs::hex_to_array<32>(std::string_view(account).substr(prefix.size()), true);
	}

	std::optional<verified_identity> verify(
		const cert& c,
		const std::vector<augs::ed25519_public_key_type>& broker_keys,
		const int64_t unix_now,
		const uint8_t* const connection_binding,
		const uint8_t* const session_signature
	) {
		if (connection_binding == nullptr || session_signature == nullptr) {
			return std::nullopt;
		}

		const auto message = signed_message(c);

		auto out = std::optional<verified_identity>();

		for (const auto& broker_key : broker_keys) {
			if (augs::ed25519_verify(broker_key, message, c.signature)) {
				const bool valid_now =
					unix_now + clock_margin_secs >= c.not_before
					&& unix_now - clock_margin_secs <= c.not_after
				;

				if (!valid_now) {
					return std::nullopt;
				}

				out = verified_identity { c.account, signer_type::BROKER };
				break;
			}
		}

		if (!out.has_value()) {
			/*
				Self-signed: only ever for the account of the key that signed it.
			*/

			const auto own_key = key_of_key_account(c.account);

			if (!own_key.has_value() || !augs::ed25519_verify(*own_key, message, c.signature)) {
				return std::nullopt;
			}

			out = verified_identity { c.account, signer_type::SELF };
		}

		auto signature = augs::ed25519_signature_type();
		std::memcpy(signature.data(), session_signature, signature.size());

		const auto binding = std::string_view(reinterpret_cast<const char*>(connection_binding), 32);

		if (!augs::ed25519_verify(c.session_public_key, binding, signature)) {
			return std::nullopt;
		}

		return out;
	}

	std::optional<blob_type> write_blob(const connect_blob& in) {
		auto out = blob_type();
		out.fill(0);

		const auto cert_bytes = in.identity.has_value() ? to_bytes(*in.identity) : std::vector<std::byte>();

		if (in.password.size() > max_password_length) {
			return std::nullopt;
		}

		if (1 + 2 + cert_bytes.size() + 1 + in.password.size() > out.size()) {
			return std::nullopt;
		}

		std::size_t i = 0;

		out[i++] = format_version;
		out[i++] = static_cast<uint8_t>(cert_bytes.size() & 0xff);
		out[i++] = static_cast<uint8_t>((cert_bytes.size() >> 8) & 0xff);

		std::memcpy(out.data() + i, cert_bytes.data(), cert_bytes.size());
		i += cert_bytes.size();

		out[i++] = static_cast<uint8_t>(in.password.size());
		std::memcpy(out.data() + i, in.password.data(), in.password.size());

		return out;
	}

	std::optional<connect_blob> read_blob(const uint8_t* const data) {
		if (data == nullptr) {
			return std::nullopt;
		}

		/*
			A zeroed blob - e.g. a loopback client - is a player with nothing to show.
		*/

		if (data[0] == 0) {
			return connect_blob();
		}

		if (data[0] != format_version) {
			return std::nullopt;
		}

		std::size_t i = 1;

		const auto cert_size = static_cast<std::size_t>(data[i]) | (static_cast<std::size_t>(data[i + 1]) << 8);
		i += 2;

		if (i + cert_size + 1 > blob_bytes) {
			return std::nullopt;
		}

		auto out = connect_blob();

		if (cert_size > 0) {
			out.identity = from_bytes(reinterpret_cast<const std::byte*>(data + i), cert_size);

			if (!out.identity.has_value()) {
				return std::nullopt;
			}
		}

		i += cert_size;

		const auto password_size = static_cast<std::size_t>(data[i]);
		i += 1;

		if (password_size > max_password_length || i + password_size > blob_bytes) {
			return std::nullopt;
		}

		out.password.assign(reinterpret_cast<const char*>(data + i), password_size);

		return out;
	}
}

#if BUILD_UNIT_TESTS && BUILD_NETWORKING
#include <Catch/single_include/catch2/catch.hpp>

namespace {
	augs::ed25519_seed_type test_seed(const int first) {
		auto out = augs::ed25519_seed_type();

		for (std::size_t i = 0; i < out.size(); ++i) {
			out[i] = static_cast<uint8_t>((static_cast<int>(i) + first) & 0xff);
		}

		return out;
	}

	std::vector<std::byte> from_hex(const std::string& hex) {
		auto out = std::vector<std::byte>();

		for (std::size_t i = 0; i + 1 < hex.size(); i += 2) {
			out.push_back(static_cast<std::byte>(std::stoi(hex.substr(i, 2), nullptr, 16)));
		}

		return out;
	}
}

/*
	The same vectors as the broker's test on hypersomnia.io (identity_cert.js), so both sides agree on every byte.
*/

TEST_CASE("Identity cert matches the broker's format") {
	const auto broker_seed = test_seed(1);
	const auto session_seed = test_seed(100);
	const auto account_seed = test_seed(50);

	const auto broker_public_key = *augs::ed25519_public_key(broker_seed);
	const auto session_public_key = *augs::ed25519_public_key(session_seed);
	const auto account_public_key = *augs::ed25519_public_key(account_seed);

	REQUIRE(augs::bytes_to_hex(broker_public_key) == "79b5562e8fe654f94078b112e8a98ba7901f853ae695bed7e0e3910bad049664");
	REQUIRE(augs::bytes_to_hex(session_public_key) == "0bbc346a57667c380120bd9c7fd7e51d2c5fdfea37cd2f5bf405b2c6bf6f2d78");
	REQUIRE(augs::bytes_to_hex(account_public_key) == "5e423033044f56a13a686799487bcbfa63dd1e39204cec27dd39a26daa615628");

	auto c = identity_cert::cert();
	c.account = "steam_76561198000000000";
	c.session_public_key = session_public_key;
	c.not_before = 1700000000;
	c.not_after = 1700007200;
	c.signature = *augs::ed25519_sign(broker_seed, identity_cert::signed_message(c));

	const auto expected = std::string(
		"0117737465616d5f37363536313139383030303030303030300bbc346a57667c380120bd9c7fd7e51d2c5fdfea37cd2f5bf405b2c6bf6f2d78"
		"00f1536500000000200d546500000000301a73c6e6345b51e50cfa48fb92972004431f34f3c8f37077b6c5c91b9c4cb9de5a0606bf0ffa59746a"
		"9908dcca455125b897be83444ee99374135bc0136c00"
	);

	REQUIRE(augs::bytes_to_hex(identity_cert::to_bytes(c)) == expected);

	const auto parsed_bytes = from_hex(expected);
	const auto parsed = identity_cert::from_bytes(parsed_bytes.data(), parsed_bytes.size());

	REQUIRE(parsed.has_value());
	REQUIRE(*parsed == c);

	const auto request_signature = augs::ed25519_sign(account_seed, identity_cert::cert_request_message(session_public_key));

	REQUIRE(identity_cert::steam_ticket_identity(session_public_key) == "b29051783415063ee1f6dd30673ca7");

	REQUIRE(augs::bytes_to_hex(*request_signature) == 
		"e905b4fedce090504a28a8d9e369701dca70e59ef41520da384af5d5f0a387af86a950f9a5c57d0aad494121e75736a44436bcdf94eeccbca9cd674d9cc02a03"
	);
}

TEST_CASE("Identity cert verification") {
	const auto broker_seed = test_seed(1);
	const auto session_seed = test_seed(100);
	const auto account_seed = test_seed(50);
	const auto other_seed = test_seed(7);

	const auto brokers = std::vector<augs::ed25519_public_key_type> { *augs::ed25519_public_key(broker_seed) };
	const auto session_public_key = *augs::ed25519_public_key(session_seed);

	auto binding = std::array<uint8_t, 32>();
	binding.fill(0x42);

	const auto binding_view = std::string_view(reinterpret_cast<const char*>(binding.data()), binding.size());
	const auto session_signature = *augs::ed25519_sign(session_seed, binding_view);
	const auto foreign_signature = *augs::ed25519_sign(other_seed, binding_view);

	auto broker_cert = identity_cert::cert();
	broker_cert.account = "steam_76561198000000000";
	broker_cert.session_public_key = session_public_key;
	broker_cert.not_before = 1700000000;
	broker_cert.not_after = 1700007200;
	broker_cert.signature = *augs::ed25519_sign(broker_seed, identity_cert::signed_message(broker_cert));

	SECTION("A broker cert within its validity") {
		const auto v = identity_cert::verify(broker_cert, brokers, 1700003600, binding.data(), session_signature.data());
		REQUIRE(v.has_value());
		REQUIRE(v->account == "steam_76561198000000000");
		REQUIRE(v->signer == identity_cert::signer_type::BROKER);
	}

	SECTION("Within the clock margin, and not past it") {
		REQUIRE(identity_cert::verify(broker_cert, brokers, 1700007200 + 599, binding.data(), session_signature.data()).has_value());
		REQUIRE(!identity_cert::verify(broker_cert, brokers, 1700007200 + 601, binding.data(), session_signature.data()).has_value());
		REQUIRE(identity_cert::verify(broker_cert, brokers, 1700000000 - 599, binding.data(), session_signature.data()).has_value());
		REQUIRE(!identity_cert::verify(broker_cert, brokers, 1700000000 - 601, binding.data(), session_signature.data()).has_value());
	}

	SECTION("The session key must have signed this connection") {
		REQUIRE(!identity_cert::verify(broker_cert, brokers, 1700003600, binding.data(), foreign_signature.data()).has_value());

		auto other_binding = binding;
		other_binding[0] ^= 1;
		REQUIRE(!identity_cert::verify(broker_cert, brokers, 1700003600, other_binding.data(), session_signature.data()).has_value());
	}

	SECTION("A changed account voids the broker's signature, and a steam account can not be self-signed") {
		auto forged = broker_cert;
		forged.account = "steam_76561198000000001";
		REQUIRE(!identity_cert::verify(forged, brokers, 1700003600, binding.data(), session_signature.data()).has_value());
	}

	SECTION("A self-signed cert is only ever for its own key, and has no validity") {
		const auto self = identity_cert::make_self_signed(account_seed, session_public_key);
		REQUIRE(self.has_value());
		REQUIRE(self->not_before == 0);
		REQUIRE(self->not_after == 0);

		const auto v = identity_cert::verify(*self, brokers, 1700003600, binding.data(), session_signature.data());
		REQUIRE(v.has_value());
		REQUIRE(v->signer == identity_cert::signer_type::SELF);
		REQUIRE(v->account == identity_cert::key_account(*augs::ed25519_public_key(account_seed)));

		auto claims_another_key = *self;
		claims_another_key.account = identity_cert::key_account(*augs::ed25519_public_key(other_seed));
		REQUIRE(!identity_cert::verify(claims_another_key, brokers, 1700003600, binding.data(), session_signature.data()).has_value());

		REQUIRE(!identity_cert::verify(*self, {}, 1700003600, binding.data(), foreign_signature.data()).has_value());
	}

	SECTION("The blob round trip") {
		auto blob = identity_cert::connect_blob();
		blob.identity = broker_cert;
		blob.password = "secret";

		const auto written = identity_cert::write_blob(blob);
		REQUIRE(written.has_value());

		const auto read = identity_cert::read_blob(written->data());
		REQUIRE(read.has_value());
		REQUIRE(read->identity.has_value());
		REQUIRE(*read->identity == broker_cert);
		REQUIRE(read->password == "secret");

		auto zero = identity_cert::blob_type();
		zero.fill(0);
		const auto empty = identity_cert::read_blob(zero.data());
		REQUIRE(empty.has_value());
		REQUIRE(!empty->identity.has_value());
		REQUIRE(empty->password.empty());

		auto garbage = *written;
		garbage[1] = 0xff;
		garbage[2] = 0xff;
		REQUIRE(!identity_cert::read_blob(garbage.data()).has_value());

		auto too_long = identity_cert::connect_blob();
		too_long.password = std::string(identity_cert::max_password_length + 1, 'x');
		REQUIRE(!identity_cert::write_blob(too_long).has_value());
	}
}
#endif
