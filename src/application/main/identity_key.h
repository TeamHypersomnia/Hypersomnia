#pragma once
#include <cstddef>
#include <optional>
#include <string>
#include <vector>
#include "augs/filesystem/path.h"
#include "augs/misc/ed25519.h"

/*
	The accounts of the players without Steam.

	The game keeps an Ed25519 key in the user folder (identity.key), created the first time it is needed.
	The account is "key_" followed by the public key in hex.

	To authenticate, the client signs the same identity a Steam ticket is requested for -
	the server's per-connection challenge and the id of the official server its connect token was issued for -
	so a signature is good for that one connection only. Like every authentication, it only goes over a secure connection.
*/

namespace identity_key {
	inline constexpr std::string_view file_name = "identity.key";
	inline constexpr std::string_view export_prefix = "hypersomnia-key-v1:";

	std::string signed_message(const std::string& auth_identity);

	/*
		Signed to link the key to a Steam account on hypersomnia.io with a one-time code shown there.
	*/

	std::string link_message(const std::string& code);

	/*
		The account id without the "key_" provider prefix.
	*/

	std::string account_id(const augs::ed25519_public_key_type& public_key);

	struct ticket {
		augs::ed25519_public_key_type public_key;
		augs::ed25519_signature_type signature;
	};

	std::vector<std::byte> to_bytes(const ticket&);
	std::optional<ticket> from_bytes(const std::vector<std::byte>&);

	std::optional<ticket> sign(const augs::ed25519_seed_type& seed, const std::string& auth_identity);
	bool verify(const ticket&, const std::string& auth_identity);

	/*
		The text the key file holds, also what "Export key" copies: "hypersomnia-key-v1:" and the seed in hex.
	*/

	std::string to_export_string(const augs::ed25519_seed_type& seed);
	std::optional<augs::ed25519_seed_type> from_export_string(const std::string& text);

	std::optional<augs::ed25519_seed_type> load(const augs::path_type& path);

	/*
		Loads the key, or creates and saves a new one (owner read/write only) if the file does not exist yet.
		std::nullopt if the file exists but is not a valid key - it is never overwritten.
	*/

	std::optional<augs::ed25519_seed_type> load_or_create(const augs::path_type& path);

	/*
		Saves an imported key. An existing key file is first renamed to identity.key.bak-<timestamp>, never deleted.
		Returns the backup's path (empty if there was nothing to back up), or std::nullopt on failure.
	*/

	std::optional<augs::path_type> save_replacing(const augs::path_type& path, const augs::ed25519_seed_type& seed);

	/*
		Forgets the key, so that a new one - a new account - can be created.
		The file is only renamed to identity.key.bak-<timestamp>, never deleted: a lost key can never be recovered.
		Returns the backup's path, or std::nullopt on failure.
	*/

	std::optional<augs::path_type> forget(const augs::path_type& path);
}
