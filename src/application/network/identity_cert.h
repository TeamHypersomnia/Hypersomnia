#pragma once
#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <vector>
#include "augs/misc/ed25519.h"

/*
	Who a player is, proven inside the connection handshake.

	A cert binds an account to the player's session key: the account key itself (.keys/account.key).
	The identity broker on hypersomnia.io signs certs after checking a Steam ticket, a CrazyGames token
	or the signature of an account key. A player can sign one with the account key himself - the account is then
	always "key_" with that very key. Servers take such a cert everywhere, ranked included: a key_ account is
	whoever holds the key; bans of such accounts are applied by hypersomnia.io when it rates the match.

	netcode signs the binding of each connection (its server key, secret and cookie) with the session key,
	so a cert is good only together with a connection its owner made. See netcode_connection_binding.

	The handshake carries, sealed to the server key, a blob in the netcode user data: the cert and the server password.
*/

namespace identity_cert {
	inline constexpr std::string_view cert_tag = "hs-cert-v1";
	inline constexpr std::string_view cert_request_tag = "hs-cert-req-v1";

	inline constexpr uint8_t format_version = 1;
	inline constexpr std::size_t max_account_length = 80;
	inline constexpr std::size_t max_password_length = 64;

	/*
		How far a server's clock may be off from the broker's either way before a broker cert stops being accepted.
	*/

	inline constexpr int64_t clock_margin_secs = 10 * 60;

	struct cert {
		std::string account;
		augs::ed25519_public_key_type session_public_key = {};

		/*
			Both zero in a self-signed cert: it has no validity, so a wrong clock never spoils it.
		*/

		int64_t not_before = 0;
		int64_t not_after = 0;

		augs::ed25519_signature_type signature = {};

		bool operator==(const cert&) const = default;
	};

	/*
		version (1), account length (1), account, session public key (32),
		not_before and not_after (8 each, little endian), signature (64).
	*/

	std::vector<std::byte> to_bytes(const cert&);
	std::optional<cert> from_bytes(const std::byte* data, std::size_t size);

	/*
		What the signer signs: the tag, then the bytes without the signature.
	*/

	std::string signed_message(const cert&);

	std::string key_account(const augs::ed25519_public_key_type& account_public_key);

	std::optional<cert> make_self_signed(
		const augs::ed25519_seed_type& account_seed,
		const augs::ed25519_public_key_type& session_public_key
	);

	/*
		What an account key signs to ask the broker for a cert: the tag and the session public key.
	*/

	std::string cert_request_message(const augs::ed25519_public_key_type& session_public_key);

	/*
		What the game requests a Steam Web API ticket for, so the ticket is good only with this session key:
		the first 15 bytes of SHA-256 of "hs-steam-v1" and the key, in hex. Steam takes an identity of at most 31 characters.
	*/

	std::string steam_ticket_identity(const augs::ed25519_public_key_type& session_public_key);

	enum class signer_type {
		BROKER,
		SELF
	};

	struct verified_identity {
		std::string account;
		signer_type signer = signer_type::SELF;
	};

	/*
		Checks the cert's signature - by one of the broker keys, or for a "key_" account by its own key -
		its validity for a broker cert, and that the session key signed this connection's binding.
	*/

	std::optional<verified_identity> verify(
		const cert&,
		const std::vector<augs::ed25519_public_key_type>& broker_keys,
		int64_t unix_now,
		const uint8_t* connection_binding,
		const uint8_t* session_signature
	);

	struct connect_blob {
		std::optional<cert> identity;
		std::string password;
	};

	inline constexpr std::size_t blob_bytes = 960;

	using blob_type = std::array<uint8_t, blob_bytes>;

	/*
		version (1), cert length (2, little endian; 0 for none), cert, password length (1), password.
	*/

	std::optional<blob_type> write_blob(const connect_blob&);
	std::optional<connect_blob> read_blob(const uint8_t* data);
}
