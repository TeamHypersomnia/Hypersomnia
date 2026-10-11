#pragma once
#include <string_view>
#include "augs/log.h"
#include "augs/filesystem/path.h"
#include "augs/misc/ed25519.h"
#include "augs/misc/secure_hash.h"
#include "application/main/identity_key.h"

/*
	Every server's Ed25519 key (server.key in the user folder), created on the first start and never overwritten.
	The clients seal their handshakes to it, and the masterserver and hypersomnia.io know an official server by it.
	The same format as the account keys - see identity_key.h.
*/

namespace server_key {
	inline constexpr std::string_view file_name = "server.key";

	/*
		A server that can not read its key file still starts - with a key for this run only,
		so it is not known as the server it was. The file is left as it is.
	*/

	inline augs::ed25519_seed_type obtain(const augs::path_type& path) {
		if (const auto seed = identity_key::load_or_create(path)) {
			return *seed;
		}

		LOG("ERROR: could not read or create the server key at %x. Using a key for this run only.", path);

		if (const auto seed = augs::ed25519_generate_seed()) {
			return *seed;
		}

		return {};
	}

	inline std::string public_key_hex(const augs::ed25519_seed_type& seed) {
		if (const auto public_key = augs::ed25519_public_key(seed)) {
			return augs::bytes_to_hex(*public_key);
		}

		return "";
	}
}
