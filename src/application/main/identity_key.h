#pragma once
#include <cstddef>
#include <optional>
#include <string>
#include <vector>
#include "augs/filesystem/path.h"
#include "augs/misc/ed25519.h"

/*
	The accounts of the players without Steam and outside CrazyGames.

	The game keeps an Ed25519 key in the user folder (account.key), created on the first start.
	The account is "key_" followed by the public key in hex - unless the key is linked to a Steam account on hypersomnia.io,
	in which case the identity broker vouches for that Steam account instead. See identity_session.h.

	The same file format serves the servers' keys (server.key) - see server_key.h.
*/

namespace identity_key {
	inline constexpr std::string_view file_name = "account.key";
	inline constexpr std::string_view export_prefix = "hypersomnia-key-v1:";

	/*
		Signed to link the key to a Steam account on hypersomnia.io with a one-time code shown there.
		The signature also names the Steam account (e.g. "steam_7656..."), the one the player was shown and agreed to.
	*/

	std::string link_message(const std::string& code, const std::string& steam_account);

	/*
		Signed to unlink the key from whatever Steam account it is linked to, from the game itself -
		e.g. after linking it with a code someone else gave. unix_time keeps an old request from being reused later.
	*/

	std::string unlink_message(const augs::ed25519_public_key_type& public_key, int64_t unix_time);

	/*
		The account id without the "key_" provider prefix.
	*/

	std::string account_id(const augs::ed25519_public_key_type& public_key);

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
		Saves an imported key. An existing key file is first renamed to account.key.bak-<timestamp>, never deleted.
		Returns the backup's path (empty if there was nothing to back up), or std::nullopt on failure.
	*/

	std::optional<augs::path_type> save_replacing(const augs::path_type& path, const augs::ed25519_seed_type& seed);

	/*
		Forgets the key, so that a new one - a new account - can be created.
		The file is only renamed to account.key.bak-<timestamp>, never deleted: a lost key can never be recovered.
		Returns the backup's path, or std::nullopt on failure.
	*/

	std::optional<augs::path_type> forget(const augs::path_type& path);
}
