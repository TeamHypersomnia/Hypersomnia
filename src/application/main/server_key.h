#pragma once
#include <cstdint>
#include <cstdlib>
#include <optional>
#include <stdexcept>
#include <string>
#include <sodium.h>
#include <string_view>
#include "augs/log.h"
#include "augs/filesystem/path.h"
#include "augs/misc/ed25519.h"
#include "augs/misc/secure_hash.h"
#include "application/main/identity_key.h"

/*
	Every server's Ed25519 key: one secret, server.key in KEYS_DIR (see all_paths.h), created on the first start and never
	overwritten - one per host, shared by all its instances (they differ by their labels - see server_instance_label.h).
	The clients seal their handshakes to that key, and the masterserver and hypersomnia.io know an official server by it.
	The same format as the account keys - see identity_key.h.

	The HYPERSOMNIA_SERVER_PRIVATE_KEY environment variable, when set, is the key instead of the file (which is then neither read
	nor made) - the same one line as in server.key, for containers without a mounted user folder.
*/

namespace server_key {
	inline constexpr std::string_view file_name = "server.key";
	inline constexpr const char* env_var_name = "HYPERSOMNIA_SERVER_PRIVATE_KEY";

	inline std::optional<std::string> from_environment() {
		if (const auto value = std::getenv(env_var_name); value != nullptr && *value != '\0') {
			return std::string(value);
		}

		return std::nullopt;
	}

	/*
		Where the key comes from - for the error messages.
	*/

	inline std::string source_name(const augs::path_type& path) {
		if (from_environment().has_value()) {
			return env_var_name;
		}

		return path.string();
	}

	inline std::string damaged_message(const augs::path_type& path) {
		if (from_environment().has_value()) {
			return std::string(env_var_name) + " is not a server key - it should be the one line of a server.key: hypersomnia-key-v1:<64 hex digits>.";
		}

		return path.string() + " is there but can not be read. Restore it, or move it away to make a new key.";
	}

	inline std::optional<augs::ed25519_seed_type> load_or_create(const augs::path_type& path) {
		if (const auto text = from_environment()) {
			return identity_key::from_export_string(*text);
		}

		return identity_key::load_or_create(path);
	}

	/*
		The game with a key file it can not read makes a new key quietly (the old file kept next to it, see
		identity_key::save_replacing) - its servers are new ones from then on. A dedicated server does not start then
		(see damaged). A damaged key in the environment variable is never replaced - hosting uses a key for this run only.
	*/

	inline augs::ed25519_seed_type obtain(const augs::path_type& path) {
		if (const auto seed = server_key::load_or_create(path)) {
			return *seed;
		}

		const auto seed = augs::ed25519_generate_seed();

		/*
			Never a made-up key (e.g. all zeros - the same for every such server, its private key known to all).
		*/

		if (!seed.has_value()) {
			throw std::runtime_error("Could not generate a server key.");
		}

		if (!from_environment().has_value() && identity_key::save_replacing(path, *seed).has_value()) {
			LOG("The server key at %x could not be read - made a new one.", path);
			return *seed;
		}

		LOG("ERROR: could not read or create the server key at %x. Using a key for this run only.", server_key::source_name(path));
		return *seed;
	}

	/*
		There, but not readable (damaged, no permission): a dedicated server does not start then - on a key for this run only
		it would not be the server it was (e.g. no longer official).
	*/

	inline bool damaged(const augs::path_type& path) {
		if (const auto text = from_environment()) {
			return !identity_key::from_export_string(*text).has_value();
		}

		return augs::exists(path) && !identity_key::load(path).has_value();
	}

	inline std::string public_key_hex(const augs::ed25519_seed_type& seed) {
		if (const auto public_key = augs::ed25519_public_key(seed)) {
			return augs::bytes_to_hex(*public_key);
		}

		return "";
	}
}
