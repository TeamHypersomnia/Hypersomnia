#pragma once
#include <stdexcept>
#include <string_view>
#include "augs/log.h"
#include "augs/filesystem/path.h"
#include "augs/misc/ed25519.h"
#include "augs/misc/secure_hash.h"
#include "application/main/identity_key.h"

/*
	Every server's Ed25519 key (server.key in KEYS_DIR - see all_paths.h), created on the first start and never overwritten.
	The clients seal their handshakes to it, and the masterserver and hypersomnia.io know an official server by it.
	The same format as the account keys - see identity_key.h.
*/

namespace server_key {
	inline constexpr std::string_view file_name = "server.key";

	/*
		Hosting from the game with a key file it can not read still starts - with a key for this run only, so it is not known
		as the server it was; the file is left as it is. A dedicated server does not start then (see damaged).
	*/

	inline augs::ed25519_seed_type obtain(const augs::path_type& path) {
		if (const auto seed = identity_key::load_or_create(path)) {
			return *seed;
		}

		LOG("ERROR: could not read or create the server key at %x. Using a key for this run only.", path);

		if (const auto seed = augs::ed25519_generate_seed()) {
			return *seed;
		}

		/*
			Never a made-up key (e.g. all zeros - the same for every such server, its private key known to all).
		*/

		throw std::runtime_error("Could not generate a server key.");
	}

	/*
		There, but not readable (damaged, no permission): a dedicated server does not start then - on a key for this run only
		it would not be the server it was (e.g. no longer official).
	*/

	inline bool damaged(const augs::path_type& path) {
		return augs::exists(path) && !identity_key::load(path).has_value();
	}

	inline std::string public_key_hex(const augs::ed25519_seed_type& seed) {
		if (const auto public_key = augs::ed25519_public_key(seed)) {
			return augs::bytes_to_hex(*public_key);
		}

		return "";
	}
}
