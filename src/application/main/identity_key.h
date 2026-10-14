#pragma once
#include <cstddef>
#include <optional>
#include <string>
#include <vector>
#include "augs/filesystem/path.h"
#include "augs/misc/ed25519.h"

/*
	The accounts of the players without Steam and outside CrazyGames.

	The game keeps an Ed25519 key in KEYS_DIR (account.key - see all_paths.h), created on the first start (in the Steam build only the session key of its Steam certs).
	The account is "key_" followed by the public key in hex - unless the key is linked to a Steam account on hypersomnia.io,
	in which case the identity broker vouches for that Steam account instead. See identity_session.h.

	The same file format serves the servers' keys (.keys/server.key) - see server_key.h.
*/

namespace identity_key {
	/*
		All keys and certs in one place, apart from the rest of the user folder.
	*/

	inline constexpr std::string_view keys_dir_name = ".keys";
	inline constexpr std::string_view file_name = "account.key";
	inline constexpr std::string_view export_prefix = "hypersomnia-key-v1:";

	/*
		Signed to link the key to a Steam account on hypersomnia.io: the game's random state for this linking and when it
		started (the page is good for a while only) - the Steam account is whoever confirms the page opened with it.
	*/

	std::string link_message(const std::string& state, int64_t unix_time);

	/*
		The account id without the "key_" provider prefix.
	*/

	std::string account_id(const augs::ed25519_public_key_type& public_key);

	/*
		The one public form of a key everywhere - menu, Settings, leaderboards, modals and the site:
		"0x" and the first 8 hex digits of the public key, e.g. "0x8aed41f2..." - the 0x says it is a key.
		For a "key_..." account id the same; any other account id is returned as it is.
	*/

	std::string short_form(const augs::ed25519_public_key_type& public_key);
	std::string short_form_of_account(const std::string& account);

	/*
		What a key's account is called to the player: it lives on this computer, or in this browser.
	*/

	inline std::string account_kind_name() {
#if PLATFORM_WEB
		return "Browser account";
#else
		return "Key account";
#endif
	}

	inline std::string account_kind_name_lowercase() {
#if PLATFORM_WEB
		return "browser account";
#else
		return "key account";
#endif
	}

	/*
		The text the key file holds, also what a downloaded backup holds: "hypersomnia-key-v1:" and the seed in hex.
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
		Saves an imported key. An existing key file is first renamed to account.key.bak-<timestamp>, never deleted -
		except in the browser, where nobody could reach a backup: replaced there.
		Returns the backup's path (empty if there was nothing to back up, or in the browser), or std::nullopt on failure.
	*/

	std::optional<augs::path_type> save_replacing(const augs::path_type& path, const augs::ed25519_seed_type& seed);

	/*
		Forgets the key, so that a new one - a new account - can be created.
		The file is only renamed to account.key.bak-<timestamp>, never deleted: a lost key can never be recovered.
		In the browser it is removed (the player was told to download it first). Returns the backup's path
		(the key's own in the browser), or std::nullopt on failure.
	*/

	std::optional<augs::path_type> forget(const augs::path_type& path);

	/*
		Writes the key to a file of the player's choosing (Backup - Save a copy...), owner-readable only.
	*/

	bool save_copy(const augs::path_type& path, const augs::ed25519_seed_type& seed);
}
